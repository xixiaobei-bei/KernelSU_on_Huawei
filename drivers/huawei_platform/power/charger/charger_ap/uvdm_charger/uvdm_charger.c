/*
 * uvdm_charger.c
 *
 * uvdm charger with sc (switch cap) driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include <huawei_platform/power/uvdm_charger/uvdm_charger.h>
#include <huawei_platform/power/wired_channel_switch.h>
#include <huawei_platform/power/power_mesg.h>
#include <huawei_platform/usb/hw_usb.h>
#include <huawei_platform/power/battery_voltage.h>
#include <huawei_platform/power/huawei_battery_temp.h>
#include <huawei_platform/usb/hw_pd_dev.h>

/* for charge temp hysteresis */
#define UVDM_LOW_TEMP_MAX          10
#define UVDM_HIGH_TEMP_MAX         45

#define HWLOG_TAG uvdm_charge
HWLOG_REGIST();

static struct direct_charge_device *g_uvdm_di;
static struct uvdm_batinfo_ops *g_bi_sc_ops;
static struct uvdm_switchcap_ops *g_sc_ops;
static struct direct_charge_volt_para_group g_sc_volt_para[DC_VOLT_GROUP_MAX];
static u8 g_cover_af_key[UVDM_RANDOM_ADD_HASH_LEN];
static bool g_uvdm_sc_enable;
static int g_otg_flag;
static int g_cover_abnormal_flag;
static bool g_bucktosc_flag;
static unsigned int g_limit_current_by_cover_temp;

static const char * const g_uvdm_stage[UVDM_STAGE_END] = {
	[UVDM_STAGE_DEFAULT] = "uvdm_stage_default",
	[UVDM_STAGE_ADAPTER_CHECK] = "uvdm_stage_adapter_check",
	[UVDM_STAGE_SWITCH_PATH] = "uvdm_stage_switch_path",
	[UVDM_STAGE_CHARGE_INIT] = "uvdm_stage_charge_init",
	[UVDM_STAGE_SECURITY_CHECK] = "uvdm_stage_security_check",
	[UVDM_STAGE_SUCCESS] = "uvdm_stage_success",
	[UVDM_STAGE_CHARGING] = "uvdm_stage_charging",
	[UVDM_STAGE_CHARGE_DONE] = "uvdm_stage_charge_done",
};

struct atomic_notifier_head g_uvdm_fault_notifier_list;
ATOMIC_NOTIFIER_HEAD(g_uvdm_fault_notifier_list);

static void uvdm_charge_stop_charging(void);

int uvdm_sc_batinfo_ops_register(struct uvdm_batinfo_ops *ops)
{
	if (!ops) {
		hwlog_err("uvdm sc batinfo ops register failed\n");
		return -EINVAL;
	}

	g_bi_sc_ops = ops;
	return 0;
}

int uvdm_sc_ops_register(struct uvdm_switchcap_ops *ops)
{
	if (!ops) {
		hwlog_err("uvdm sc batinfo ops register failed\n");
		return -EINVAL;
	}

	g_sc_ops = ops;
	return 0;
}

static void uvdm_charge_set_stage_status(unsigned int stage)
{
	struct direct_charge_device *l_di = g_uvdm_di;

	if (!l_di)
		return;

	l_di->dc_stage = stage;
	hwlog_info("set_stage_status: stage=%d\n", l_di->dc_stage);
}

static unsigned int uvdm_charge_get_stage_status(void)
{
	struct direct_charge_device *l_di = g_uvdm_di;

	if (!l_di)
		return UVDM_STAGE_DEFAULT;

	return l_di->dc_stage;
}

int uvdm_charge_in_charging_stage(void)
{
	if (uvdm_charge_get_stage_status() >= UVDM_STAGE_ADAPTER_CHECK &&
		uvdm_charge_get_stage_status() <= UVDM_STAGE_CHARGING)
		return UVDM_IN_CHARGING_STAGE;

	return UVDM_NOT_IN_CHARGING_STAGE;
}

static int uvdm_set_iin_limit(unsigned int val)
{
	struct direct_charge_device *di = g_uvdm_di;
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

	hwlog_info("set input current is:%u, limit current is %d\n",
		val, di->sysfs_iin_thermal);
	return 0;
}

static int uvdm_get_iin_limit(unsigned int *val)
{
	struct direct_charge_device *di = g_uvdm_di;

	if (!di || !val) {
		hwlog_err("di or val is null\n");
		return -1;
	}

	*val = di->sysfs_iin_thermal;
	return 0;
}

static struct power_if_ops uvdm_if_ops = {
	.set_iin_limit = uvdm_set_iin_limit,
	.get_iin_limit = uvdm_get_iin_limit,
	.type_name = "uvdm",
};

static void uvdm_set_sc_enable(bool flag)
{
	hwlog_info("%s value = %d\n", __func__, flag);
	g_uvdm_sc_enable = flag;
}

bool uvdm_get_sc_enable(void)
{
	return g_uvdm_sc_enable;
}

static void uvdm_charge_set_stop_charging_flag(int value)
{
	struct direct_charge_device *l_di = g_uvdm_di;

	hwlog_info("uvdm set stop charging flag = %d\n", value);
	l_di->stop_charging_flag_error = value;
}

static void uvdm_otg_schedule_charge_monitor_work(void)
{
	struct charge_device_info *di = huawei_charge_get_di();

	if (!di)
		return;

	mod_delayed_work(system_wq, &di->charge_work, msecs_to_jiffies(0));
}

void uvdm_handle_cover_report_event(void)
{
	if (g_otg_flag == UVDM_OTG_PLUG_OUT) {
		g_otg_flag = -1;
		uvdm_schedule_work();
	}

	if (g_cover_abnormal_flag == UVDM_ABNORMAL_TYPE_TEMP_RECOVERY) {
		g_cover_abnormal_flag = -1;
		uvdm_schedule_work();
	}
}

static void uvdm_charge_send_super_charging_uevent(void)
{
	struct direct_charge_device *l_di = g_uvdm_di;

	/* set super_charging flag */
	l_di->super_charge_flag = 1;
	direct_charge_connect_send_uevent();
}

int uvdm_charge_get_super_charging_flag(void)
{
	struct direct_charge_device *l_di = g_uvdm_di;

	if (!l_di)
		return 0;

	return l_di->super_charge_flag;
}

int uvdm_charge_cutoff_reset_param(void)
{
	struct direct_charge_device *di = g_uvdm_di;

	if (!di)
		return -1;

	di->full_path_resistance = DC_ERROR_RESISTANCE;
	di->dc_succ_flag = DC_ERROR_ADAPTER_DETECT;
	di->scp_stop_charging_flag_info = 0;
	di->cur_stage = 0;
	di->pre_stage = 0;
	di->vbat = 0;
	di->ibat = 0;
	di->vadapt = 0;
	di->iadapt = 0;
	di->ls_vbus = 0;
	di->ls_ibus = 0;
	di->compensate_v = 0;
	di->ibat_abnormal_cnt = 0;
	di->dc_open_retry_cnt = 0;
	di->dc_err_report_flag = false;
	di->low_temp_hysteresis = 0;
	di->high_temp_hysteresis = 0;
	di->bat_temp_err_flag = false;
	di->error_cnt = 0;
	di->super_charge_flag = 0;
	g_otg_flag = -1;
	g_cover_abnormal_flag = -1;
	g_limit_current_by_cover_temp = UVDM_MAX_CURRENT;
	uvdm_charge_set_stop_charging_flag(0);
	uvdm_charge_set_stage_status(UVDM_STAGE_DEFAULT);
	uvdm_set_sc_enable(false);

	return 0;
}

static const char *uvdm_charge_get_stage_status_string(unsigned int stage)
{
	if ((stage >= UVDM_STAGE_BEGIN) && (stage < UVDM_STAGE_END))
		return g_uvdm_stage[stage];

	return "illegal stage status";
}

static int uvdm_dc_get_protocol_register_state(void)
{
	if (adapter_get_protocol_register_state(ADAPTER_PROTOCOL_UVDM)) {
		hwlog_err("adapter protocol not ready\n");
		return -1;
	}

	uvdm_charge_set_stage_status(UVDM_STAGE_SWITCH_PATH);
	return 0;
}

static void uvdm_set_antifake_result(unsigned int data)
{
	struct direct_charge_device *di = g_uvdm_di;

	di->dc_antifake_result = data;
	complete(&di->dc_af_completion);

	hwlog_info("antifake_result=%d\n", di->dc_antifake_result);
}

static int uvdm_get_antifake_data(char *buf, unsigned int len)
{
	int i;

	for (i = 0; i < len; i++) {
		buf[i] = g_cover_af_key[i];
		g_cover_af_key[i] = 0;
	}

	return len;
}

#ifdef CONFIG_SYSFS
static ssize_t uvdm_charge_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t uvdm_charge_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info uvdm_charge_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(uvdm_charge, 0644, UVDM_CHARGE_SYSFS_KEY_ID, kid),
	power_sysfs_attr_rw(uvdm_charge, 0644, UVDM_CHARGE_SYSFS_AF, af),
};

#define UVDM_CHARGE_SYSFS_ATTRS_SIZE  ARRAY_SIZE(uvdm_charge_sysfs_field_tbl)

static struct attribute *uvdm_charge_sysfs_attrs[UVDM_CHARGE_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group uvdm_charge_sysfs_attr_group = {
	.attrs = uvdm_charge_sysfs_attrs,
};

static void uvdm_charge_sysfs_init_attrs(void)
{
	int i;

	for (i = 0; i < UVDM_CHARGE_SYSFS_ATTRS_SIZE; i++)
		uvdm_charge_sysfs_attrs[i] = &uvdm_charge_sysfs_field_tbl[i].attr.attr;

	uvdm_charge_sysfs_attrs[i] = NULL;
}

static struct power_sysfs_attr_info *uvdm_charge_sysfs_field_lookup(
	const char *name)
{
	int i;

	for (i = 0; i < UVDM_CHARGE_SYSFS_ATTRS_SIZE; i++) {
		if (!strncmp(name, uvdm_charge_sysfs_field_tbl[i].attr.attr.name,
			strlen(name)))
			break;
	}

	if (i >= UVDM_CHARGE_SYSFS_ATTRS_SIZE)
		return NULL;

	return &uvdm_charge_sysfs_field_tbl[i];
}

static ssize_t uvdm_charge_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct direct_charge_device *di = dev_get_drvdata(dev);

	info = uvdm_charge_sysfs_field_lookup(attr->attr.name);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case UVDM_CHARGE_SYSFS_KEY_ID:
		return snprintf(buf, PAGE_SIZE, "%d\n", di->adp_antifake_key_index);
	case UVDM_CHARGE_SYSFS_AF:
		return uvdm_get_antifake_data(buf, UVDM_RANDOM_ADD_HASH_LEN);
	default:
		return 0;
	}
}

static ssize_t uvdm_charge_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct direct_charge_device *di = dev_get_drvdata(dev);
	long val = 0;

	info = uvdm_charge_sysfs_field_lookup(attr->attr.name);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case UVDM_CHARGE_SYSFS_KEY_ID:
		/* 10: decimal base, must be 0 or 1 */
		if ((kstrtol(buf, 10, &val) < 0) || (val < 0) || (val > 1))
			return -EINVAL;

		di->adp_antifake_key_index = val;
		hwlog_info("adp_antifake_key_index=%d\n", di->adp_antifake_key_index);
		break;
	case UVDM_CHARGE_SYSFS_AF:
		/* 10: decimal base, must be 0 or 1 */
		if ((kstrtol(buf, 10, &val) < 0) || (val < 0) || (val > 1))
			return -EINVAL;

		uvdm_set_antifake_result(val);
		break;
	default:
		break;
	}
	return count;
}

static void uvdm_charge_sysfs_create_group(struct device *dev)
{
	uvdm_charge_sysfs_init_attrs();
	power_sysfs_create_link_group("hw_power", "charger", "uvdm_charger",
		dev, &uvdm_charge_sysfs_attr_group);
}

static void uvdm_charge_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "uvdm_charger",
		dev, &uvdm_charge_sysfs_attr_group);
}
#else
static inline void uvdm_charge_sysfs_create_group(struct device *dev)
{
}

static inline void uvdm_charge_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int uvdm_charge_wait_adapter_antifake(void)
{
	struct direct_charge_device *di = g_uvdm_di;

	if (!wait_for_completion_timeout(&di->dc_af_completion,
		UVDM_AF_WAIT_CT_TIMEOUT)) {
		hwlog_err("bms_auth service wait timeout\n");
		return -1;
	}
	reinit_completion(&di->dc_af_completion);

	/*
	 * if not timeout,
	 * return the antifake result base on the hash calc result
	 */
	if (di->dc_antifake_result == 0) {
		hwlog_err("bms_auth hash calculate fail\n");
		return -1;
	}

	hwlog_info("bms_auth hash calculate ok\n");
	return 0;
}

static int uvdm_dc_check_adapter_antifake(void)
{
	int ret;
	struct direct_charge_device *l_di = g_uvdm_di;
	struct power_event_notify_data n_data;

	if (!l_di->adp_antifake_enable)
		return 0;

	memset(g_cover_af_key, 0x00, UVDM_RANDOM_ADD_HASH_LEN);
	ret = adapter_auth_encrypt_start(ADAPTER_PROTOCOL_UVDM,
		l_di->adp_antifake_key_index, g_cover_af_key, UVDM_RANDOM_ADD_HASH_LEN);
	if (ret) {
		hwlog_err("%s: get hash from tx fail\n", __func__);
		return ret;
	}

	l_di->dc_antifake_result = 0;
	n_data.event = "AUTH_WCCT_UVDM=";
	n_data.event_len = UVDM_CHARGE_EVENT_LEN;
	power_event_notify(POWER_EVENT_NE_AUTH_WL_SC_UVDM, &n_data);
	ret = uvdm_charge_wait_adapter_antifake();
	if (ret) {
		hwlog_err("%s: antifake fail\n", __func__);
		return ret;
	}

	return ret;
}

static int uvdm_charge_get_stop_charging_flag(void)
{
	struct direct_charge_device *l_di = g_uvdm_di;

	if (l_di->stop_charging_flag_error) {
		hwlog_info("stop_charging_flag_error is set\n");
		return -1;
	}

	return 0;
}

static int uvdm_charge_get_bat_current(int *value)
{
	if (!g_bi_sc_ops->get_bat_current)
		return -1;

	if (g_bi_sc_ops->get_bat_current(value)) {
		uvdm_charge_set_stop_charging_flag(1);
		*value = 0;
		hwlog_err("get battery current fail\n");
		return -1;
	}

	hwlog_info("cali_b: bat_cur=%d\n", *value);
	return 0;
}

static int uvdm_charge_get_bat_voltage(int *value)
{
	int btb_vol;
	int package_vol;

	if (!g_bi_sc_ops->get_bat_btb_voltage)
		return -1;

	if (!g_bi_sc_ops->get_bat_package_voltage)
		return -1;

	btb_vol = g_bi_sc_ops->get_bat_btb_voltage();
	package_vol = g_bi_sc_ops->get_bat_package_voltage();
	if (btb_vol < 0 || package_vol < 0) {
		uvdm_charge_set_stop_charging_flag(1);
		*value = 0;
		hwlog_err("get battery voltage fail\n");
		return -1;
	}

	*value = (btb_vol < package_vol) ? btb_vol : package_vol;
	return 0;
}

static int uvdm_charge_get_device_ibus(int *value)
{
	if (!g_bi_sc_ops->get_ls_ibus)
		return -1;

	if (g_bi_sc_ops->get_ls_ibus(value)) {
		uvdm_charge_set_stop_charging_flag(1);
		*value = 0;
		hwlog_err("get sc device ibus fail\n");
		return -1;
	}

	return 0;
}

static int uvdm_charge_get_device_vbus(int *value)
{
	if (!g_bi_sc_ops->get_vbus_voltage)
		return -1;

	if (g_bi_sc_ops->get_vbus_voltage(value)) {
		uvdm_charge_set_stop_charging_flag(1);
		*value = 0;
		hwlog_err("get sc device vbus fail\n");
		return -1;
	}

	return 0;
}

static int uvdm_charge_get_device_temp(int *value)
{
	if (!g_bi_sc_ops->get_ls_temp)
		return -1;

	if (g_bi_sc_ops->get_ls_temp(value)) {
		uvdm_charge_set_stop_charging_flag(1);
		*value = 0;
		hwlog_err("get sc device temp fail\n");
		return -1;
	}

	return 0;
}

static int uvdm_charge_get_adapter_voltage(int *value)
{
	if (uvdm_charge_get_stop_charging_flag())
		return -1;

	if (adapter_get_output_voltage(ADAPTER_PROTOCOL_UVDM, value)) {
		uvdm_charge_set_stop_charging_flag(1);
		*value = 0;
		hwlog_err("get adapter output voltage fail\n");
		return -1;
	}

	return 0;
}

static int uvdm_charge_set_voltage(int value)
{
	struct direct_charge_device *l_di = g_uvdm_di;
	int l_value = value;

	uvdm_charge_get_device_vbus(&l_di->ls_vbus);
	if (l_di->ls_vbus < UVDM_VBUS_THRESHOLD) {
		uvdm_charge_set_stop_charging_flag(1);
		hwlog_err("vbus is lower than the threshold\n");
		return -1;
	}

	if (uvdm_charge_get_stop_charging_flag())
		return -1;

	hwlog_info("set adapter_volt=%d,max_volt=%d, vbus=%d\n",
		l_value, l_di->max_adapter_vset, l_di->ls_vbus);

	if (l_value > l_di->max_adapter_vset) {
		l_value = l_di->max_adapter_vset;
		l_di->adaptor_vset = l_di->max_adapter_vset;
	}

	if (adapter_set_output_voltage(ADAPTER_PROTOCOL_UVDM, l_value)) {
		uvdm_charge_set_stop_charging_flag(1);
		hwlog_err("set adapter voltage fail\n");
		return -1;
	}

	return 0;
}

static int uvdm_charge_set_voltage_step_by_step(int end)
{
	int vol = STEP_VOL_START;
	int step = 3000; /* step in 3000mv */
	int cnt = 10; /* max times */

	if (!g_sc_ops->ls_enable_prepare)
		return uvdm_charge_set_voltage(end);

	if (uvdm_charge_set_voltage(vol))
		return -1;

	if (g_sc_ops->ls_enable_prepare())
		return -1;

	while (cnt--) {
		vol += step;
		hwlog_info("vol step open:%d\n", vol);

		if (vol < end) {
			if (uvdm_charge_set_voltage(vol))
				return -1;
			usleep_range(10000, 10200); /* sleep 10ms */
		} else {
			return uvdm_charge_set_voltage(end);
		}
	}

	return -1;
}

static int uvdm_charge_switch_charging_path(unsigned int path)
{
	switch (path) {
	case UVDM_PATH_BUCK:
		charge_set_hiz_enable(HIZ_MODE_DISABLE);
		hwlog_info("switch buck path success\n");
		return 0;
	case UVDM_PATH_SC:
		charge_set_hiz_enable(HIZ_MODE_ENABLE);
		hwlog_info("switch sc path success\n");
		return 0;
	default:
		hwlog_info("unknown path\n");
		return -1;
	}
}

/* get the maximum current allowed by uvdm charging at specified temp */
static int uvdm_charge_battery_temp_handler(int value)
{
	int i;
	struct direct_charge_device *l_di = g_uvdm_di;

	for (i = 0; i < DC_TEMP_LEVEL; ++i) {
		if (value >= l_di->temp_para[i].temp_min &&
			value < l_di->temp_para[i].temp_max)
			return l_di->temp_para[i].temp_cur_max;
	}

	hwlog_err("current temp is illegal, temp=%d\n", value);
	return 0;
}

static int uvdm_charge_check_battery_temp(void)
{
	int bat_temp = 0;
	int bat_temp_cur_max;
	struct direct_charge_device *l_di = g_uvdm_di;

	huawei_battery_temp(BAT_TEMP_MIXED, &bat_temp);
	if (bat_temp < UVDM_LOW_TEMP_MAX + l_di->low_temp_hysteresis) {
		hwlog_info("can not do uvdm charging again, temp=%d\n", bat_temp);
		return -1;
	}
	if (bat_temp >= UVDM_HIGH_TEMP_MAX - l_di->high_temp_hysteresis) {
		hwlog_info("can not do uvdm charging again, temp=%d\n", bat_temp);
		return -1;
	}

	bat_temp_cur_max = uvdm_charge_battery_temp_handler(bat_temp);
	if (bat_temp_cur_max == 0) {
		hwlog_info("can not do uvdm charging, temp=%d\n", bat_temp);
		return -1;
	}

	return 0;
}

static int uvdm_charge_check_battery_voltage(void)
{
	int volt_max = hw_battery_voltage(BAT_ID_MAX);
	int volt_min = hw_battery_voltage(BAT_ID_MIN);
	struct direct_charge_device *l_di = g_uvdm_di;

	if (volt_min < l_di->min_dc_bat_vol ||
		volt_max > l_di->max_dc_bat_vol) {
		hwlog_info("can not do uvdm charging, v_min=%d v_max=%d\n",
			volt_min, volt_max);
		return -1;
	}

	return 0;
}

static int uvdm_charge_check_cover_voltage_accuracy(void)
{
	int vadapt = 0;
	int vol_err;
	int i;
	int retry = 3; /* retry 3 times */
	int error_flag = 0;
	struct direct_charge_device *l_di = g_uvdm_di;

	l_di->adaptor_vset = l_di->init_adapter_vset;
	uvdm_charge_set_voltage(l_di->adaptor_vset);

	/* delay 500ms */
	usleep_range(500000, 501000);

	for (i = 0; i < retry; ++i) {
		if (uvdm_charge_get_adapter_voltage(&vadapt))
			return -1;

		vol_err = vadapt - l_di->adaptor_vset;
		if (vol_err < 0)
			vol_err = -vol_err;

		hwlog_info("[%d]: Verr=%d, Verr_th=%d, Vset=%d, Vread=%d, Vbus=%d\n",
			i, vol_err, l_di->vol_err_th, l_di->adaptor_vset,
			vadapt, get_charger_vbus_vol());

		if (vol_err > l_di->vol_err_th) {
			error_flag = 1;
			break;
		}
	}

	if (error_flag == 0) {
		hwlog_err("adapter voltage accuracy check succ\n");
		return 0;
	}

	hwlog_err("adapter voltage accuracy check fail\n");
	return -1;
}

static int uvdm_adjust_voltage_until_current_rise(
	struct direct_charge_device *l_di)
{
	int vbat = 0;
	int ibus = 0;
	int vbat_max;
	int vbat_max_temp = hisi_battery_vbat_max();
	int adjust_times = MAX_TIMES_FOR_SET_ADAPTER_VOL;
	int soc;

	if (g_sc_ops->ls_enable(1)) {
		hwlog_err("ls enable fail\n");
		goto fail_open;
	}

	/* delay 10ms */
	usleep_range(10000, 11000);

	/* adjust adapter voltage until current to 800ma */
	if (uvdm_charge_get_device_ibus(&ibus))
		return -1;

	vbat_max = vbat_max_temp > l_di->max_dc_bat_vol ?
		vbat_max_temp : l_di->max_dc_bat_vol;

	hwlog_info("[%d]: Vapt_set=%d, Ibus_ls=%d Vbat_max=%d\n",
		adjust_times, l_di->adaptor_vset, ibus, vbat_max);

	while (ibus < MIN_CURRENT_FOR_RES_DETECT) {
		if (g_otg_flag == UVDM_OTG_PLUG_IN) {
			uvdm_set_sc_enable(false);
			return -1;
		}

		uvdm_charge_get_bat_voltage(&vbat);
		if (vbat_max < vbat) {
			hwlog_err("vbat too high, Vapt_set=%d\n", vbat);
			goto fail_open;
		}

		if (adjust_times-- == 0) {
			hwlog_err("try too many times, Ibus_ls=%d\n", ibus);
			goto fail_open;
		}

		l_di->adaptor_vset += l_di->vstep;
		uvdm_charge_set_voltage(l_di->adaptor_vset);
		/* delay 800ms */
		usleep_range(800000, 810000);
		uvdm_charge_get_device_ibus(&ibus);
		hwlog_info("[%d]: Vapt_set=%d, Ibus_ls=%d\n",
			adjust_times, l_di->adaptor_vset, ibus);
	}

	hwlog_info("charging path open succ\n");
	return 0;

fail_open:
	soc = hisi_battery_capacity();
	if (soc >= BATTERY_CAPACITY_HIGH_TH) {
		l_di->dc_open_retry_cnt += 1;
		hwlog_err("current battery capacity is %d, over threshold %d\n",
		soc, BATTERY_CAPACITY_HIGH_TH);
	}

	hwlog_err("charging path open fail\n");
	return -1;
}

static void uvdm_charge_init_adaptor_vbus(struct direct_charge_device *l_di)
{
	int vbat = 0;

	uvdm_charge_get_bat_voltage(&vbat);
	l_di->adaptor_vset = vbat * l_di->dc_volt_ratio + l_di->init_delt_vset;
	if (l_di->max_adapter_vset < l_di->adaptor_vset)
		l_di->adaptor_vset = l_di->max_adapter_vset;
	uvdm_charge_set_voltage_step_by_step(l_di->adaptor_vset);
}

static int uvdm_charge_open_charging_path(void)
{
	struct direct_charge_device *l_di = g_uvdm_di;

	if (!g_sc_ops->ls_enable) {
		hwlog_err("sc enable failed\n");
		return -1;
	}
	uvdm_charge_init_adaptor_vbus(l_di);

	/* delay 50ms */
	msleep(50);

	return uvdm_adjust_voltage_until_current_rise(l_di);
}

static int uvdm_charge_security_check(void)
{
	struct direct_charge_device *l_di = g_uvdm_di;

	l_di->dc_succ_flag = UVDM_ERROR_ADAPTER_VOLTAGE_ACCURACY;

	if (uvdm_charge_check_cover_voltage_accuracy())
		return -1;

	l_di->dc_succ_flag = UVDM_ERROR_OPEN_CHARGE_PATH;
	if (uvdm_charge_open_charging_path())
		return -1;

	return 0;
}

static void uvdm_charge_select_charging_volt_param(void)
{
	int i;
	char *brand = NULL;
	int tbat = 0;
	struct direct_charge_device *l_di = g_uvdm_di;

	brand = hisi_battery_brand();
	huawei_battery_temp(BAT_TEMP_MIXED, &tbat);

	hwlog_info("stage_group_size=%d, bat_brand=%s, bat_temp=%d\n",
		l_di->stage_group_size, brand, tbat);

	for (i = 0; i < l_di->stage_group_size; i++) {
		if (!l_di->orig_volt_para_p[i].bat_info.parse_ok)
			continue;

		if (!strstr(brand, l_di->orig_volt_para_p[i].bat_info.batid))
			continue;

		if ((tbat >= l_di->orig_volt_para_p[i].bat_info.temp_high) ||
			(tbat < l_di->orig_volt_para_p[i].bat_info.temp_low))
			continue;

		l_di->stage_group_cur = i;
		l_di->stage_size = l_di->orig_volt_para_p[i].stage_size;
		memcpy(l_di->volt_para,
			l_di->orig_volt_para_p[i].volt_info,
			sizeof(l_di->volt_para));
		memcpy(l_di->orig_volt_para,
			l_di->orig_volt_para_p[i].volt_info,
			sizeof(l_di->orig_volt_para));

		hwlog_info("[%d]: bat_id=%s, temp_low=%d, temp_high=%d\n", i,
			l_di->orig_volt_para_p[i].bat_info.batid,
			l_di->orig_volt_para_p[i].bat_info.temp_low,
			l_di->orig_volt_para_p[i].bat_info.temp_high);
		return;
	}

	l_di->stage_group_cur = 0;
	l_di->stage_size = l_di->orig_volt_para_p[0].stage_size;
	memcpy(l_di->volt_para, l_di->orig_volt_para_p[0].volt_info,
		sizeof(l_di->volt_para));
	memcpy(l_di->orig_volt_para, l_di->orig_volt_para_p[0].volt_info,
		sizeof(l_di->orig_volt_para));
}

static int uvdm_charge_jump_stage_if_need(int cur_stage)
{
	int i;
	struct direct_charge_device *l_di = g_uvdm_di;

	/* 2: cc & cv */
	for (i = 0; i < 2 * l_di->stage_size; ++i) {
		if (l_di->stage_need_to_jump[i] == -1)
			return cur_stage;

		if (cur_stage == l_di->stage_need_to_jump[i]) {
			hwlog_info("jump stage to %d\n", cur_stage);
			return uvdm_charge_jump_stage_if_need(cur_stage + 1);
		}
	}

	return cur_stage;
}

static void uvdm_charge_select_charging_stage(void)
{
	int i;
	int vbat_th;
	int cur_stage = 0;
	int ibus = 0;
	int vbus = 0;
	struct direct_charge_device *di = g_uvdm_di;

	uvdm_charge_get_bat_voltage(&di->vbat);
	uvdm_charge_get_bat_current(&di->ibat);
	uvdm_charge_get_device_ibus(&ibus);
	uvdm_charge_get_device_vbus(&vbus);
	if (ibus < di->ibat_abnormal_th)
		di->ibat_abnormal_cnt++;
	else
		di->ibat_abnormal_cnt = 0;

	/* try atmost 10 times */
	if (di->ibat_abnormal_cnt > 10) {
		uvdm_charge_set_stop_charging_flag(1);
		hwlog_err("ibat_abnormal_cnt is more than ten times");
		return;
	}

	di->pre_stage = di->cur_stage;
	for (i = di->stage_size - 1; i >= 0; --i) {
		vbat_th = di->volt_para[i].vol_th + di->compensate_v;
		vbat_th = (vbat_th > di->volt_para[di->stage_size - 1].vol_th) ?
			di->volt_para[di->stage_size - 1].vol_th : vbat_th;
		/* 0: cc stage, 1: cv stage, 2: max stage */
		if (di->vbat >= vbat_th && di->ibat <= di->volt_para[i].cur_th_low) {
			cur_stage = 2 * i + 2;
			break;
		} else if (di->vbat >= vbat_th) {
			cur_stage = 2 * i + 1;
			break;
		}
	}

	if (i < 0)
		cur_stage = 0;

	if (cur_stage < di->pre_stage)
		cur_stage = di->pre_stage;

	if (di->first_cc_stage_timer_in_min) {
		/* 0: cc stage, 1: cv stage, 2: max stage */
		if (cur_stage == 0 || cur_stage == 1) {
			if (time_after(jiffies, di->first_cc_stage_timeout)) {
				hwlog_info("first_cc_stage in %d min, set stage=2\n",
					di->first_cc_stage_timer_in_min);

				cur_stage = 2;
			}
		}
	}

	if (cur_stage != di->cur_stage)
		di->cur_stage = uvdm_charge_jump_stage_if_need(cur_stage);
}

static int uvdm_set_disable_flags(int val, int type)
{
	int i;
	unsigned int disable = 0;
	struct direct_charge_device *di = g_uvdm_di;

	if (type < UVDM_DISABLE_BEGIN || type >= UVDM_DISABLE_END) {
		hwlog_err("invalid disable_type=%d\n", type);
		return -1;
	}

	di->sysfs_disable_charger[type] = val;
	for (i = 0; i < UVDM_DISABLE_END; i++)
		disable |= di->sysfs_disable_charger[i];
	di->sysfs_enable_charger = !disable;

	hwlog_info("set_disable_flag val=%d, type=%d, disable=%d\n",
		val, type, disable);
	return 0;
}

void uvdm_charge_select_charging_param(void)
{
	int cur_th_high;
	int vbat_th;
	int vol_th;
	int tbat = 0;
	int tbat_max_cur;
	struct direct_charge_device *l_di = g_uvdm_di;

	/* get max current with specified temp */
	huawei_battery_temp(BAT_TEMP_MIXED, &tbat);
	tbat_max_cur = uvdm_charge_battery_temp_handler(tbat);
	if (tbat_max_cur == 0) {
		hwlog_info("temp=%d, can not do uvdm charging\n", tbat);
		if (tbat < UVDM_LOW_TEMP_MAX)
			l_di->low_temp_hysteresis = l_di->orig_low_temp_hysteresis;

		if (tbat >= UVDM_HIGH_TEMP_MAX)
			l_di->high_temp_hysteresis = l_di->orig_high_temp_hysteresis;

		l_di->scp_stop_charging_flag_info = 1;
		l_di->bat_temp_err_flag = true;
	}

	/* unit: 1000mv */
	l_di->compensate_v = l_di->ibat * l_di->compensate_r / 1000;
	/* cur_stage include cc and cv stage so divide 2 */
	vbat_th = l_di->volt_para[l_di->cur_stage / 2].vol_th +
		l_di->compensate_v;
	vol_th = l_di->volt_para[l_di->stage_size - 1].vol_th;
	l_di->cur_vbat_th = vbat_th < vol_th ? vbat_th : vol_th;
	/* cur_stage include cc and cv stage so divide 2 */
	cur_th_high = l_di->volt_para[l_di->cur_stage / 2].cur_th_high;
	cur_th_high = cur_th_high > tbat_max_cur ? tbat_max_cur : cur_th_high;
	cur_th_high = cur_th_high > g_limit_current_by_cover_temp ?
		g_limit_current_by_cover_temp : cur_th_high;
	l_di->cur_ibat_th_high = cur_th_high > l_di->sysfs_iin_thermal ?
		l_di->sysfs_iin_thermal : cur_th_high;
	/* cur_stage include cc and cv stage so divide 2 */
	l_di->cur_ibat_th_low = l_di->volt_para[l_di->cur_stage / 2].cur_th_low;
}

static void uvdm_charge_regulation_core(struct direct_charge_device *di)
{
	/* cc and cv stage */
	if (di->cur_stage % 2) {
		if (di->vbat > di->cur_vbat_th) {
			di->adaptor_vset += di->dc_volt_ratio * (di->cur_vbat_th - di->vbat);
			uvdm_charge_set_voltage(di->adaptor_vset);
			return;
		}

		if (di->iadapt > di->cur_ibat_th_high / di->dc_volt_ratio) {
			di->adaptor_vset -= di->vstep;
			uvdm_charge_set_voltage(di->adaptor_vset);
			return;
		}

		if (di->ibat > di->cur_ibat_th_high) {
			di->adaptor_vset -= di->vstep;
			uvdm_charge_set_voltage(di->adaptor_vset);
			return;
		}

		if (di->ibat > di->cur_ibat_th_high - di->delta_err) {
			hwlog_info("do nothing\n");
			return;
		}

		if (di->iadapt < (di->cur_ibat_th_high - di->delta_err) / di->dc_volt_ratio) {
			di->adaptor_vset += di->vstep;
			uvdm_charge_set_voltage(di->adaptor_vset);
			return;
		}
	} else {
		if (di->iadapt > di->cur_ibat_th_high / di->dc_volt_ratio) {
			di->adaptor_vset -= di->vstep;
			uvdm_charge_set_voltage(di->adaptor_vset);
			return;
		}

		if (di->ibat > di->cur_ibat_th_high) {
			di->adaptor_vset -= di->vstep;
			uvdm_charge_set_voltage(di->adaptor_vset);
			return;
		}

		if (di->ibat > di->cur_ibat_th_high - di->delta_err) {
			hwlog_info("do nothing\n");
			return;
		}

		if (di->iadapt < (di->cur_ibat_th_high - di->delta_err) / di->dc_volt_ratio) {
			di->adaptor_vset += di->vstep;
			uvdm_charge_set_voltage(di->adaptor_vset);
			return;
		}
	}
}

static void uvdm_charge_regulation(void)
{
	struct direct_charge_device *l_di = g_uvdm_di;

	uvdm_charge_get_device_ibus(&l_di->ls_ibus);
	uvdm_charge_get_device_vbus(&l_di->ls_vbus);
	uvdm_charge_get_device_temp(&l_di->tls);
	l_di->iadapt = l_di->ls_ibus;

	hwlog_info("cur_stage[%d]: vbat=%d vbat_th=%d\t"
		"ibat=%d ibat_th_high=%d\t"
		"ls_vbus=%d ls_ibus=%d tls=%d\n",
		l_di->cur_stage, l_di->vbat, l_di->cur_vbat_th,
		l_di->ibat, l_di->cur_ibat_th_high,
		l_di->ls_vbus, l_di->ls_ibus, l_di->tls);

	/* check temperature with device */
	if (l_di->tls > l_di->max_tls) {
		uvdm_charge_set_stop_charging_flag(1);
		return;
	}

	uvdm_charge_regulation_core(l_di);
}

static void uvdm_charge_wake_lock(void)
{
	struct direct_charge_device *l_di = g_uvdm_di;

	if (!l_di->charging_lock.active) {
		__pm_stay_awake(&l_di->charging_lock);
		hwlog_info("wake lock\n");
	}
}

static void uvdm_charge_wake_unlock(void)
{
	struct direct_charge_device *l_di = g_uvdm_di;

	if (l_di->charging_lock.active) {
		__pm_relax(&l_di->charging_lock);
		hwlog_info("wake unlock\n");
	}
}

void uvdm_charge_control_work(struct work_struct *work)
{
	struct direct_charge_device *di = NULL;
	int interval;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct direct_charge_device, control_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	if (di->stop_charging_flag_error ||
		di->scp_stop_charging_flag_info ||
		(di->sysfs_enable_charger == 0)) {
		hwlog_info("uvdm charge stop\n");
		uvdm_charge_stop_charging();
		return;
	}

	if (DOUBLE_SIZE * di->stage_size == di->cur_stage) {
		hwlog_info("cur_stage=%d, vbat=%d, ibat=%d\n",
			di->cur_stage, di->vbat, di->ibat);
		hwlog_info("uvdm charge done\n");
		uvdm_charge_set_stage_status(UVDM_STAGE_CHARGE_DONE);
		uvdm_charge_stop_charging();
		return;
	}

	uvdm_charge_regulation();
	interval = di->charge_control_interval;
	hrtimer_start(&di->control_timer,
		ktime_set(interval / MSEC_PER_SEC,
			(interval % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);
}

void uvdm_charge_calc_thld_work(struct work_struct *work)
{
	struct direct_charge_device *di = NULL;
	int interval;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct direct_charge_device, calc_thld_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	if (di->stop_charging_flag_error ||
		di->scp_stop_charging_flag_info ||
		(di->sysfs_enable_charger == 0)) {
		hwlog_info("uvdm charge stop, stop calc threshold\n");
		return;
	}

	uvdm_charge_select_charging_stage();
	uvdm_charge_select_charging_param();

	if (DOUBLE_SIZE * di->stage_size == di->cur_stage) {
		hwlog_info("uvdm charge done, stop calc threshold\n");
		return;
	}

	interval = di->threshold_caculation_interval;
	hrtimer_start(&di->calc_thld_timer,
		ktime_set(interval / MSEC_PER_SEC,
			(interval % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);
}

enum hrtimer_restart uvdm_charge_calc_thld_timer_func(struct hrtimer *timer)
{
	struct direct_charge_device *di = NULL;

	if (!timer) {
		hwlog_err("timer is null\n");
		return HRTIMER_NORESTART;
	}

	di = container_of(timer, struct direct_charge_device, calc_thld_timer);
	if (!di) {
		hwlog_err("di is null\n");
		return HRTIMER_NORESTART;
	}

	queue_work(di->charging_wq, &di->calc_thld_work);
	return HRTIMER_NORESTART;
}

enum hrtimer_restart uvdm_charge_control_timer_func(struct hrtimer *timer)
{
	struct direct_charge_device *di = NULL;

	if (!timer) {
		hwlog_err("timer is null\n");
		return HRTIMER_NORESTART;
	}

	di = container_of(timer, struct direct_charge_device, control_timer);
	if (!di) {
		hwlog_err("di is null\n");
		return HRTIMER_NORESTART;
	}

	queue_work(di->charging_wq, &di->control_work);
	return HRTIMER_NORESTART;
}

static void uvdm_charge_set_rx_reduce_voltage(void)
{
	if (uvdm_charge_get_stage_status() == UVDM_STAGE_CHARGE_DONE ||
		g_cover_abnormal_flag == UVDM_ABNORMAL_TYPE_OTP_TO5W) {
		hwlog_info("set rx reduce voltage\n");
		uvdm_set_sc_enable(false);
		adapter_set_rx_reduce_voltage(ADAPTER_PROTOCOL_UVDM);
	}
}

static void uvdm_charge_reset_para_in_stop(void)
{
	struct direct_charge_device *di = g_uvdm_di;

	di->error_cnt = 0;
	di->dc_open_retry_cnt = 0;
	di->dc_succ_flag = UVDM_ERROR_ADAPTER_DETECT;
	di->super_charge_flag = 0;
	di->dc_err_report_flag = false;
	di->low_temp_hysteresis = 0;
	di->high_temp_hysteresis = 0;
	uvdm_charge_set_stage_status(UVDM_STAGE_DEFAULT);
	uvdm_set_sc_enable(false);
}

static void uvdm_charge_stop_charging_preaction(void)
{
	struct direct_charge_device *l_di = g_uvdm_di;

	if (l_di->stop_charging_flag_error)
		l_di->error_cnt += 1;

	if (l_di->stop_charging_flag_error ||
		l_di->scp_stop_charging_flag_info ||
		(l_di->sysfs_enable_charger == 0))
		uvdm_charge_set_stage_status(UVDM_STAGE_DEFAULT);
	else
		uvdm_charge_set_stage_status(UVDM_STAGE_CHARGE_DONE);
}

static void uvdm_charge_stop_charging_restore_path_config(void)
{
	int ret;

	ret = g_sc_ops->ls_enable(0);
	if (ret)
		hwlog_err("ls enable fail\n");

	uvdm_charge_switch_charging_path(UVDM_PATH_BUCK);

	/* 1: enable discharge with device */
	if (g_sc_ops->ls_discharge) {
		ret = g_sc_ops->ls_discharge(1);
		if (ret)
			hwlog_err("ls enable discharge fail\n");
		else
			hwlog_info("ls enable discharge succ\n");
	}

	msleep(20); /* delay 20ms */
}

static void uvdm_charge_stop_charging_disconnect_check(void)
{
	int vbus = 0;
	int vbat = 0;

	/* judging whether the adapter is disconnect */
	uvdm_charge_get_device_vbus(&vbus);
	uvdm_charge_get_bat_voltage(&vbat);
	if ((vbus < VBUS_ON_THRESHOLD) ||
		(vbat - vbus) > VBAT_VBUS_DIFFERENCE) {
		hwlog_info("uvdm charger disconnect\n");
		uvdm_set_disable_flags(UVDM_CLEAR_DISABLE_FLAGS,
			UVDM_DISABLE_SYS_NODE);
		uvdm_charge_reset_para_in_stop();
		direct_charge_disconnect_send_uevent();
	} else {
		adapter_set_slave_power_mode(ADAPTER_PROTOCOL_UVDM,
			UVDM_CHARGE_SCTOBUCK5W);
	}

	if (g_sc_ops->ls_exit())
		hwlog_err("ls exit fail\n");

	if (g_bi_sc_ops->exit())
		hwlog_err("bi exit fail\n");
}

static void uvdm_charge_stop_charging_reset_config(void)
{
	struct direct_charge_device *l_di = g_uvdm_di;

	/* release resource */
	hrtimer_cancel(&l_di->calc_thld_timer);
	cancel_work_sync(&l_di->calc_thld_work);
	uvdm_charge_set_stop_charging_flag(0);
	uvdm_charge_wake_unlock();

	/* reset parameter */
	if (g_otg_flag == UVDM_OTG_PLUG_IN)
		uvdm_set_sc_enable(false);
	g_otg_flag = -1;
	g_cover_abnormal_flag = -1;
	g_limit_current_by_cover_temp = UVDM_MAX_CURRENT;
	l_di->scp_stop_charging_flag_info = 0;
	l_di->cur_stage = 0;
	l_di->pre_stage = 0;
	l_di->vbat = 0;
	l_di->ibat = 0;
	l_di->vadapt = 0;
	l_di->iadapt = 0;
	l_di->ls_vbus = 0;
	l_di->ls_ibus = 0;
	l_di->compensate_v = 0;
	l_di->ibat_abnormal_cnt = 0;
	l_di->scp_stop_charging_complete_flag = 1;
	l_di->adaptor_test_result_type = TYPE_OTHER;
	l_di->working_mode = UNDEFINED_MODE;
}

static void uvdm_charge_stop_charging(void)
{
	hwlog_info("uvdm stop charging\n");
	uvdm_charge_stop_charging_preaction();
	uvdm_charge_stop_charging_restore_path_config();
	uvdm_charge_stop_charging_disconnect_check();
	uvdm_charge_set_rx_reduce_voltage();
	uvdm_charge_stop_charging_reset_config();
}

static void uvdm_charge_start_charging(void)
{
	int interval;
	int msecs;
	struct direct_charge_device *l_di = g_uvdm_di;

	hwlog_info("uvdm start charging\n");

	/* first: request wake lock */
	uvdm_charge_wake_lock();

	/* second: calculate first cc stage timeout */
	hwlog_info("first_cc_stage time=%d\n", l_di->first_cc_stage_timer_in_min);
	if (l_di->first_cc_stage_timer_in_min) {
		/* 8a maximum 5 min, ap will not suspend, so use jiffies */
		msecs = l_di->first_cc_stage_timer_in_min * 60 * MSEC_PER_SEC;
		l_di->first_cc_stage_timeout = jiffies +
			msecs_to_jiffies(msecs);
	}

	/* third: select charging voltage parameter */
	uvdm_charge_select_charging_volt_param();

	/* fourth: select charging stage */
	uvdm_charge_select_charging_stage();
	uvdm_charge_set_stage_status(UVDM_STAGE_CHARGING);

	/* fifth: select charging parameter */
	uvdm_charge_select_charging_param();

	/* sixth: start charging control timer */
	interval = l_di->charge_control_interval;
	hrtimer_start(&l_di->control_timer,
		ktime_set(interval / MSEC_PER_SEC,
			(interval % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);

	/* seventh: start threshold calculation timer */
	interval = l_di->threshold_caculation_interval;
	hrtimer_start(&l_di->calc_thld_timer,
		ktime_set(interval / MSEC_PER_SEC,
			(interval % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);
}

static int uvdm_charge_init_device(void)
{
	if (!g_sc_ops->ls_init)
		return -1;

	if (!g_bi_sc_ops->init)
		return -1;

	if (g_sc_ops->ls_init())
		return -1;

	if (g_bi_sc_ops->init())
		return -1;

	hwlog_info("uvdm charge init succ\n");
	return 0;
}

static int uvdm_sc_mode_precheck(struct direct_charge_device *di)
{
	/* step-1:check sc error */
	if (di->error_cnt >= UVDM_ERR_CNT_MAX) {
		hwlog_info("error exceed %d times, sc is disabled\n", UVDM_ERR_CNT_MAX);
		di->dc_succ_flag = UVDM_ERROR_CHARGE_DISABLED;
		uvdm_set_sc_enable(false);
		if ((di->dc_err_report_flag == false) &&
			(di->dc_open_retry_cnt <= DC_OPEN_RETRY_CNT_MAX))
			di->dc_err_report_flag = true;
		return -1;
	}

	/* step-2:check sc enable status */
	if (di->sysfs_enable_charger == 0) {
		di->dc_succ_flag = UVDM_ERROR_CHARGE_DISABLED;
		hwlog_info("sc is disabled\n");
		return -1;
	}

	return 0;
}

static int uvdm_sc_mode_adaptor_check(struct direct_charge_device *di)
{
	if (g_bucktosc_flag) {
		hwlog_info("g_bucktosc_flag = %d\n", g_bucktosc_flag);
		uvdm_charge_set_stage_status(UVDM_STAGE_ADAPTER_CHECK);
		return 0;
	}

	/* step-1:check cover antifake */
	if (uvdm_dc_check_adapter_antifake()) {
		di->dc_succ_flag = UVDM_ERROR_ADAPTER_ANTI_FAKE;
		di->error_cnt += 1;
		hwlog_err("adapter antifake check failed\n");
		return -1;
	}

	/* step-2:set adaptor mode */
	if (adapter_set_slave_power_mode(ADAPTER_PROTOCOL_UVDM,
		UVDM_CHARGE_BUCK5WTOSC)) {
		hwlog_err("set mode failed\n");
		return -1;
	}

	if (g_otg_flag == UVDM_OTG_PLUG_IN) {
		uvdm_set_sc_enable(false);
		return -1;
	}

	uvdm_charge_set_stage_status(UVDM_STAGE_ADAPTER_CHECK);
	return 0;
}

static int uvdm_sc_mode_battery_check(struct direct_charge_device *di)
{
	/* step-1: disable PD charge */
	uvdm_set_sc_enable(true);
	/* wait 200ms */
	msleep(200);

	/* step-2: check battery temp */
	if (uvdm_charge_check_battery_temp()) {
		di->dc_succ_flag = UVDM_ERROR_BAT_TEMP;
		uvdm_charge_set_stage_status(UVDM_STAGE_DEFAULT);
		di->bat_temp_err_flag = true;
		hwlog_err("temp out of range, try next loop\n");
		return -1;
	}

	/* step-3: check battery voltage */
	if (uvdm_charge_check_battery_voltage()) {
		di->dc_succ_flag = UVDM_ERROR_BAT_VOL;
		uvdm_charge_set_stage_status(UVDM_STAGE_DEFAULT);
		hwlog_err("volt out of range, try next loop\n");
		return -1;
	}

	return 0;
}

static int uvdm_sc_mode_init_charging_path(struct direct_charge_device *di)
{
	unsigned int stage;

	/* step-1: check uvdm protocol status */
	stage = uvdm_charge_get_stage_status();
	if (stage == UVDM_STAGE_ADAPTER_CHECK) {
		if (uvdm_dc_get_protocol_register_state())
			return -1;
	}

	/* step-2: switch buck charging path to sc charging path */
	stage = uvdm_charge_get_stage_status();
	if (stage == UVDM_STAGE_SWITCH_PATH) {
		di->scp_stop_charging_complete_flag = 0;
		di->dc_succ_flag = UVDM_ERROR_SWITCH_PATH;
		if (uvdm_charge_switch_charging_path(UVDM_PATH_SC) == 0) {
			uvdm_charge_set_stage_status(UVDM_STAGE_CHARGE_INIT);
		} else {
			hwlog_err("uvdm charge path switch failed\n");
			uvdm_charge_set_stop_charging_flag(1);
			uvdm_charge_stop_charging();
			return -1;
		}
	}

	/* step-3: init device */
	stage = uvdm_charge_get_stage_status();
	if (stage == UVDM_STAGE_CHARGE_INIT) {
		di->dc_succ_flag = UVDM_ERROR_INIT;
		if (uvdm_charge_init_device() == 0) {
			uvdm_charge_set_stage_status(UVDM_STAGE_SECURITY_CHECK);
		} else {
			hwlog_err("uvdm charge init failed\n");
			uvdm_charge_set_stop_charging_flag(1);
			uvdm_charge_stop_charging();
			return -1;
		}
	}

	return 0;
}

static int uvdm_sc_mode_start_secure_charge(struct direct_charge_device *di)
{
	int stage;
	int max_pwr = UVDM_MAX_POWER;

	if (g_otg_flag == UVDM_OTG_PLUG_IN) {
		uvdm_set_sc_enable(false);
		return -1;
	}

	/* step-1: send charging event */
	uvdm_charge_send_super_charging_uevent();
	soc_decimal_event_notify(SOC_DECIMAL_NE_DC, &max_pwr);

	/* step-2: uvdm charging security check */
	stage = uvdm_charge_get_stage_status();
	if (stage == UVDM_STAGE_SECURITY_CHECK) {
		if (uvdm_charge_security_check() == 0) {
			uvdm_charge_set_stage_status(UVDM_STAGE_SUCCESS);
		} else {
			hwlog_err("uvdm_charge security check failed\n");
			uvdm_charge_set_stop_charging_flag(1);
			uvdm_charge_stop_charging();
		}
	}

	/* step-3 start uvdm charging */
	stage = uvdm_charge_get_stage_status();
	if (stage == UVDM_STAGE_SUCCESS) {
		di->dc_succ_flag = UVDM_SUCC;
		uvdm_charge_start_charging();
	}

	/* step-4 end */
	stage = uvdm_charge_get_stage_status();
	if (stage == UVDM_STAGE_CHARGING)
		hwlog_info("in uvdm_charge process\n");

	hwlog_info("uvdm_charge stage=%d,%s\n",
		stage, uvdm_charge_get_stage_status_string(stage));
	return 0;
}

static void uvdm_sc_mode_check(void)
{
	struct direct_charge_device *di = g_uvdm_di;

	if (uvdm_sc_mode_precheck(di))
		return;

	if (uvdm_sc_mode_adaptor_check(di))
		return;

	if (uvdm_sc_mode_battery_check(di))
		return;

	if (uvdm_sc_mode_init_charging_path(di))
		return;

	if (uvdm_sc_mode_start_secure_charge(di))
		return;
}

void uvdm_pre_check(void)
{
	struct direct_charge_device *di = g_uvdm_di;

	if (!di || !g_sc_ops || !g_bi_sc_ops) {
		hwlog_err("di is null\n");
		return;
	}

	if (uvdm_get_sc_enable() == false)
		return;

	hwlog_info("%s come in\n", __func__);
	if (di->dc_stage == UVDM_STAGE_CHARGE_DONE) {
		hwlog_info("sc charge done\n");
		return;
	}

	g_bucktosc_flag = true;
	uvdm_sc_mode_check();
}

void uvdm_check(int type)
{
	struct direct_charge_device *di = g_uvdm_di;

	if (!di || !g_sc_ops || !g_bi_sc_ops) {
		hwlog_err("di is null\n");
		return;
	}

	hwlog_info(" %s type=%d\n", __func__, type);

	if (type == UVDM_CHARGE_TYPE_20W) {
		g_bucktosc_flag = false;
		if (di->dc_stage == UVDM_STAGE_CHARGE_DONE) {
			hwlog_info("sc charge done\n");
			return;
		}

		if (!di->dc_err_report_flag)
			uvdm_sc_mode_check();
	}
}

static void uvdm_sc_fault_work(struct work_struct *work)
{
	struct direct_charge_device *di = NULL;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct direct_charge_device, fault_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	uvdm_charge_set_stop_charging_flag(1);

	switch (di->charge_fault) {
	case UVDM_FAULT_VBUS_OVP:
		hwlog_err("vbus ovp happened\n");
		break;
	case UVDM_FAULT_TDIE_OTP:
		hwlog_err("tdie otp happened\n");
		break;
	case UVDM_FAULT_VDROP_OVP:
		hwlog_err("vdrop ovp happened\n");
		break;
	case UVDM_FAULT_VBAT_OVP:
		hwlog_err("vbat ovp happened\n");
		break;
	case UVDM_FAULT_IBAT_OCP:
		hwlog_err("ibat ocp happened\n");
		break;
	case UVDM_FAULT_IBUS_OCP:
		hwlog_err("ibus ocp happened\n");
		break;
	default:
		hwlog_err("unknown fault: %d happened\n", di->charge_fault);
		break;
	}
}

void uvdm_charge_fault_event_notify(unsigned long event, void *data)
{
	atomic_notifier_call_chain(&g_uvdm_fault_notifier_list, event, data);
}

static int uvdm_charge_fault_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct direct_charge_device *di = NULL;

	if (!nb) {
		hwlog_err("nb is null\n");
		return NOTIFY_OK;
	}

	di = container_of(nb, struct direct_charge_device, fault_nb);
	if (!di) {
		hwlog_err("di is null\n");
		return NOTIFY_OK;
	}

	if (event == UVDM_FAULT_OTG) {
		g_otg_flag = *((int *)data);
		hwlog_info("otg event:flag = %d\n", g_otg_flag);
	} else if (event == UVDM_FAULT_COVER_ABNORMAL) {
		g_cover_abnormal_flag = *((int *)data);
		hwlog_info("abnormal event:flag = %d\n", g_cover_abnormal_flag);
	} else {
		if (uvdm_charge_in_charging_stage() == UVDM_NOT_IN_CHARGING_STAGE) {
			hwlog_err("ignore notifier when not in uvdm charging\n");
			return NOTIFY_OK;
		}
		di->charge_fault = event;
		schedule_work(&di->fault_work);
		return NOTIFY_OK;
	}

	if (g_otg_flag == UVDM_OTG_PLUG_OUT) {
		uvdm_otg_schedule_charge_monitor_work();
		return NOTIFY_OK;
	}

	if (g_cover_abnormal_flag == UVDM_ABNORMAL_TYPE_TEMP_RECOVERY) {
		uvdm_otg_schedule_charge_monitor_work();
		return NOTIFY_OK;
	}

	if (g_cover_abnormal_flag == UVDM_ABNORMAL_TYPE_OTP_TO10W) {
		g_limit_current_by_cover_temp = UVDM_LIMIT_CURRENT_BY_COVER_OTP;
		hwlog_info("limit_current = %d\n", g_limit_current_by_cover_temp);
		g_cover_abnormal_flag = -1;
		return NOTIFY_OK;
	}

	uvdm_charge_set_stop_charging_flag(1);
	return NOTIFY_OK;
}

static void uvdm_sc_init_resource(void)
{
	if (!g_uvdm_di) {
		hwlog_err("g_uvdm_di is null\n");
		return;
	}

	g_otg_flag = -1;
	g_cover_abnormal_flag = -1;
	g_bucktosc_flag = false;
	g_limit_current_by_cover_temp = UVDM_MAX_CURRENT;
	g_uvdm_di->sysfs_enable_charger = 1;
	g_uvdm_di->dc_stage = UVDM_STAGE_DEFAULT;
	g_uvdm_di->sysfs_iin_thermal = g_uvdm_di->iin_thermal_default;
	g_uvdm_di->dc_succ_flag = UVDM_ERROR_ADAPTER_DETECT;
	g_uvdm_di->scp_stop_charging_complete_flag = 1;
	g_uvdm_di->dc_err_report_flag = false;
	g_uvdm_di->bat_temp_err_flag = false;
	g_uvdm_di->error_cnt = 0;
	uvdm_set_sc_enable(false);
	g_uvdm_di->charging_wq = create_singlethread_workqueue("uvdm_charging_wq");
	wakeup_source_init(&g_uvdm_di->charging_lock, "uvdm_wakelock");
	INIT_WORK(&g_uvdm_di->calc_thld_work, uvdm_charge_calc_thld_work);
	INIT_WORK(&g_uvdm_di->control_work, uvdm_charge_control_work);
	INIT_WORK(&g_uvdm_di->fault_work, uvdm_sc_fault_work);

	hrtimer_init(&g_uvdm_di->calc_thld_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	g_uvdm_di->calc_thld_timer.function = uvdm_charge_calc_thld_timer_func;
	hrtimer_init(&g_uvdm_di->control_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	g_uvdm_di->control_timer.function = uvdm_charge_control_timer_func;
	init_completion(&g_uvdm_di->dc_af_completion);
}

static int uvdm_sc_probe(struct platform_device *pdev)
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
	di->orig_volt_para_p = g_sc_volt_para;
	ret = direct_charge_parse_dts(np, di);
	if (ret)
		goto fail_free_mem;

	g_uvdm_di = di;
	uvdm_sc_init_resource();
	platform_set_drvdata(pdev, di);
	uvdm_charge_sysfs_create_group(di->dev);

	g_uvdm_di->fault_nb.notifier_call = uvdm_charge_fault_notifier_call;
	ret = atomic_notifier_chain_register(&g_uvdm_fault_notifier_list,
		&g_uvdm_di->fault_nb);
	if (ret < 0) {
		hwlog_err("register notifier failed\n");
		goto fail_register;
	}

	power_if_ops_register(&uvdm_if_ops);
	return 0;

fail_register:
	uvdm_charge_sysfs_remove_group(di->dev);
	wakeup_source_trash(&di->charging_lock);
fail_free_mem:
	devm_kfree(&pdev->dev, di);
	g_uvdm_di = NULL;
	return ret;
}

static int uvdm_sc_remove(struct platform_device *pdev)
{
	struct direct_charge_device *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	uvdm_charge_sysfs_remove_group(di->dev);
	wakeup_source_trash(&di->charging_lock);
	devm_kfree(&pdev->dev, di);
	g_uvdm_di = NULL;

	return 0;
}

static const struct of_device_id uvdm_sc_match_table[] = {
	{
		.compatible = "uvdm_charger",
		.data = NULL,
	},
	{},
};

static struct platform_driver uvdm_sc_driver = {
	.probe = uvdm_sc_probe,
	.remove = uvdm_sc_remove,
	.driver = {
		.name = "uvdm_charger",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(uvdm_sc_match_table),
	},
};

static int __init uvdm_sc_init(void)
{
	return platform_driver_register(&uvdm_sc_driver);
}

static void __exit uvdm_sc_exit(void)
{
	platform_driver_unregister(&uvdm_sc_driver);
}

late_initcall(uvdm_sc_init);
module_exit(uvdm_sc_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("uvdm charger with switch cap module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
