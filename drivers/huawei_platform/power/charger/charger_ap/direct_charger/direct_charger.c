/*
 * direct_charger.c
 *
 * direct charger driver
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

#include <linux/random.h>
#include <huawei_platform/power/direct_charger/direct_charger.h>
#include <huawei_platform/power/wired_channel_switch.h>
#include <huawei_platform/power/power_mesg.h>
#include <huawei_platform/usb/hw_usb.h>
#include <huawei_platform/power/battery_voltage.h>
#include <huawei_platform/power/huawei_battery_temp.h>
#include <huawei_platform/usb/hw_pd_dev.h>

#define HWLOG_TAG direct_charge
HWLOG_REGIST();

static BLOCKING_NOTIFIER_HEAD(dc_notify_head);

/* for ops */
static struct direct_charge_device *g_di;
struct direct_charge_pps_ops *g_pps_ops;
struct direct_charge_cd_ops *g_cd_ops;

static unsigned int g_dc_mode;
static bool g_dc_can_enter = true;

static bool g_is_abnormal_scp_adp;

/* for current calibration */
#define C_OFFSET_MAX_LEN         20
#define C_OFFSET_A_MAX           1200000
#define C_OFFSET_A_MIN           800000
static long c_offset_a;
static long c_offset_b;

/* for charge temp hysteresis */
#define DC_LOW_TEMP_MAX          10
#define DC_HIGH_TEMP_MAX         45

static const char * const g_dc_stage[DC_STAGE_END] = {
	[DC_STAGE_DEFAULT] = "dc_stage_default",
	[DC_STAGE_SUPPORT_DETECT] = "dc_stage_support_detect",
	[DC_STAGE_ADAPTER_DETECT] = "dc_stage_adapter_detect",
	[DC_STAGE_SWITCH_DETECT] = "dc_stage_switch_detect",
	[DC_STAGE_CHARGE_INIT] = "dc_stage_charge_init",
	[DC_STAGE_SECURITY_CHECK] = "dc_stage_security_check",
	[DC_STAGE_SUCCESS] = "dc_stage_success",
	[DC_STAGE_CHARGING] = "dc_stage_charging",
	[DC_STAGE_CHARGE_DONE] = "dc_stage_charge_done",
};

static const char * const g_dc_charging_path[PATH_END] = {
	[PATH_NORMAL] = "path_normal",
	[PATH_LVC] = "path_lvc",
	[PATH_SC] = "path_sc",
};

static int __init early_parse_direct_charge_cmdline(char *p)
{
	char *token = NULL;

	if (!p) {
		hwlog_err("cmdline is null\n");
		return -1;
	}

	hwlog_info("direct_charger=%s\n", p);

	token = strsep(&p, ",");
	if (token) {
		/* 10: decimal base */
		if (kstrtoul(token, 10, &c_offset_a) != 0)
			return -1;
	}

	token = strsep(&p, ",");
	if (token) {
		/* 10: decimal base */
		if (kstrtoul(token, 10, &c_offset_b) != 0)
			return -1;
	}

	hwlog_info("parse cmdline: c_offset_a=%ld,c_offset_b=%ld\n",
		c_offset_a, c_offset_b);

	if (c_offset_a < C_OFFSET_A_MIN || c_offset_a > C_OFFSET_A_MAX) {
		c_offset_a = 0;
		hwlog_err("c_offset_a invalid\n");
		return -1;
	}

	return 0;
}

early_param("direct_charger", early_parse_direct_charge_cmdline);

/* protocol power supply for direct charge */
int direct_charge_pps_ops_register(struct direct_charge_pps_ops *ops)
{
	int ret = 0;

	if (ops) {
		g_pps_ops = ops;
		hwlog_info("protocol power supply ops register ok\n");
	} else {
		hwlog_err("protocol power supply ops register fail\n");
		ret = -EINVAL;
	}

	return ret;
}

/* cable detect for direct charge */
int direct_charge_cd_ops_register(struct direct_charge_cd_ops *ops)
{
	int ret = 0;

	if (ops) {
		g_cd_ops = ops;
		hwlog_info("cable detect ops register ok\n");
	} else {
		hwlog_err("cable detect ops register fail\n");
		ret = -EINVAL;
	}

	return ret;
}

void direct_charge_get_g_pps_ops(struct direct_charge_pps_ops **ops)
{
	*ops = g_pps_ops;
}

void direct_charge_get_g_cd_ops(struct direct_charge_cd_ops **ops)
{
	*ops = g_cd_ops;
}

/* notifier chain register interface for direct charge */
int direct_charge_notifier_chain_register(struct notifier_block *nb)
{
	if (!nb) {
		hwlog_err("nb is null\n");
		return NOTIFY_OK;
	}

	return blocking_notifier_chain_register(&dc_notify_head, nb);
}

/* notifier chain register uninterface for direct charge */
int direct_charge_notifier_chain_unregister(struct notifier_block *nb)
{
	if (!nb) {
		hwlog_err("nb is null\n");
		return NOTIFY_OK;
	}

	return blocking_notifier_chain_unregister(&dc_notify_head, nb);
}

void direct_charge_set_di(struct direct_charge_device *di)
{
	if (di)
		g_di = di;
	else
		hwlog_err("di is null\n");
}

static struct direct_charge_device *direct_charge_get_di(void)
{
	if (!g_di) {
		hwlog_err("g_di is null\n");
		return NULL;
	}

	return g_di;
}

static void direct_charge_wake_lock(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	if (!l_di->charging_lock.active) {
		__pm_stay_awake(&l_di->charging_lock);
		hwlog_info("wake lock\n");
	}
}

static void direct_charge_wake_unlock(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	if (l_di->charging_lock.active) {
		__pm_relax(&l_di->charging_lock);
		hwlog_info("wake unlock\n");
	}
}

static void direct_charge_set_can_enter_status(bool status)
{
	hwlog_info("can_enter_status=%d\n", status);
	g_dc_can_enter = status;
}

static bool direct_charge_get_can_enter_status(void)
{
	return g_dc_can_enter;
}

void direct_charge_set_abnormal_adp_flag(bool flag)
{
	hwlog_info("abnormal_adp_flag=%d\n", flag);
	g_is_abnormal_scp_adp = flag;
}

bool direct_charge_get_abnormal_adp_flag(void)
{
	return g_is_abnormal_scp_adp;
}

/* get the stage of direct charge */
unsigned int direct_charge_get_stage_status(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return DC_STAGE_DEFAULT;

	return l_di->dc_stage;
}

/* get the stage of direct charge */
const char *direct_charge_get_stage_status_string(unsigned int stage)
{
	if ((stage >= DC_STAGE_BEGIN) && (stage < DC_STAGE_END))
		return g_dc_stage[stage];

	return "illegal stage_status";
}

/* set the stage of direct charge */
void direct_charge_set_stage_status(unsigned int stage)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	l_di->dc_stage = stage;

	hwlog_info("set_stage_status: stage=%d\n", l_di->dc_stage);
}

void direct_charge_set_stage_status_default(void)
{
	direct_charge_set_stage_status(DC_STAGE_DEFAULT);
}

int direct_charge_in_charging_stage(void)
{
	if (direct_charge_get_stage_status() == DC_STAGE_CHARGING)
		return DC_IN_CHARGING_STAGE;

	return DC_NOT_IN_CHARGING_STAGE;
}

int direct_charge_get_working_mode(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return UNDEFINED_MODE;

	return l_di->working_mode;
}

void direct_charge_set_local_mode(int set, unsigned int dc_mode)
{
	if (set == AND_SET)
		g_dc_mode &= dc_mode;

	if (set == OR_SET)
		g_dc_mode |= dc_mode;
}

unsigned int direct_charge_get_local_mode(void)
{
	return g_dc_mode;
}

static unsigned int direct_charge_update_local_mode(unsigned int mode)
{
	unsigned int local_mode = mode;
	struct direct_charge_device *lvc_di = NULL;
	struct direct_charge_device *sc_di = NULL;

	lvc_get_di(&lvc_di);
	sc_get_di(&sc_di);
	if (!lvc_di && !sc_di) {
		hwlog_info("local not support direct_charge\n");
		return UNDEFINED_MODE;
	}
	hwlog_info("original local mode %u\n", mode);

	if (sc_di) {
		if (sc_di->sysfs_enable_charger == 0) {
			local_mode &= ~SC_MODE;
			hwlog_info("disable sc mode, local_mode %u\n",
				local_mode);
		}
	}
	if (lvc_di) {
		if (lvc_di->sysfs_enable_charger == 0) {
			local_mode &= ~LVC_MODE;
			hwlog_info("disable lvc mode, local_mode %u\n",
				local_mode);
		}
	}

	return local_mode;
}

int direct_charge_is_failed(void)
{
	struct direct_charge_device *lvc_di = NULL;
	struct direct_charge_device *sc_di = NULL;

	lvc_get_di(&lvc_di);
	sc_get_di(&sc_di);

	if (direct_charge_get_can_enter_status() == false) {
		hwlog_info("can not enter dc\n");
		return 1;
	}

	if (lvc_di) {
		if (lvc_di->error_cnt >= DC_ERR_CNT_MAX) {
			hwlog_info("lvc error exceed retry max\n");
			return 1;
		} else {
			return 0;
		}
	}

	if (sc_di) {
		if (sc_di->error_cnt >= DC_ERR_CNT_MAX) {
			hwlog_info("sc error exceed retry max\n");
			return 1;
		} else {
			return 0;
		}
	}

	return 0;
}

void direct_charge_set_disable_flags(int val, int type)
{
	lvc_set_disable_flags(val, type);
	sc_set_disable_flags(val, type);
}

int direct_charge_get_info(enum direct_charge_info_type type, int *value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di || !value)
		return -1;

	switch (type) {
	case CC_CABLE_DETECT_OK:
		if (l_di->cc_cable_detect_enable &&
			l_di->cc_cable_detect_ok == 0)
			*value = 0;
		else
			*value = 1;
		return 0;
	default:
		return -1;
	}
}

void direct_charge_send_normal_charging_uevent(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	/* clear quick_charging and super_charging flag */
	l_di->quick_charge_flag = 0;
	l_di->super_charge_flag = 0;

	direct_charge_connect_send_uevent();
}

void direct_charge_send_quick_charging_uevent(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	/* clear super_charging flag and set quick_charging flag */
	l_di->quick_charge_flag = 1;
	l_di->super_charge_flag = 0;

	direct_charge_connect_send_uevent();
}

static void direct_charge_send_super_charging_uevent(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	/* clear quick_charging flag and set super_charging flag */
	l_di->quick_charge_flag = 0;
	l_di->super_charge_flag = 1;

	direct_charge_connect_send_uevent();
}

int direct_charge_get_quick_charging_flag(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return 0;

	return l_di->quick_charge_flag;
}

int direct_charge_get_super_charging_flag(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return 0;

	return l_di->super_charge_flag;
}

/* get the maximum current allowed by adapter at specified voltage */
static int direct_charge_adapter_volt_handler(int val,
	struct direct_charge_adp_cur_para *para)
{
	int i;

	if (!para)
		return 0;

	for (i = 0; i < DC_ADP_CUR_LEVEL; ++i) {
		if ((val >= para[i].vol_min) && (val < para[i].vol_max))
			return para[i].cur_th;
	}

	return 0;
}

/* get the maximum current allowed by direct charging at specified temp */
static int direct_charge_battery_temp_handler(int value)
{
	int i;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return 0;

	for (i = 0; i < DC_TEMP_LEVEL; ++i) {
		if (value >= l_di->temp_para[i].temp_min &&
			value < l_di->temp_para[i].temp_max)
			return l_di->temp_para[i].temp_cur_max;
	}

	hwlog_err("current temp is illegal, temp=%d\n", value);
	return 0;
}

/* get the maximum current allowed by direct charging at specified resist */
static int direct_charge_resist_handler(struct direct_charge_device *di,
	int value)
{
	int i;
	struct direct_charge_device *l_di = di;

	if (!l_di)
		return 0;

	if (pd_dpm_get_ctc_cable_flag()) {
		for (i = 0; i < DC_RESIST_LEVEL; ++i) {
			if ((value >= l_di->ctc_resist_para[i].resist_min) &&
				(value < l_di->ctc_resist_para[i].resist_max))
				return l_di->ctc_resist_para[i].resist_cur_max;
		}
	} else if (l_di->cc_cable_detect_ok == 0) {
		for (i = 0; i < DC_RESIST_LEVEL; ++i) {
			if ((value >= l_di->nonstd_resist_para[i].resist_min) &&
				(value < l_di->nonstd_resist_para[i].resist_max))
				return l_di->nonstd_resist_para[i].resist_cur_max;
		}
	} else if (l_di->cc_cable_detect_ok == 1) {
		for (i = 0; i < DC_RESIST_LEVEL; ++i) {
			if (value >= l_di->std_resist_para[i].resist_min &&
				value < l_di->std_resist_para[i].resist_max)
				return l_di->std_resist_para[i].resist_cur_max;
		}
	}

	hwlog_err("current resist is illegal, cable=%d, resist=%d\n",
		l_di->cc_cable_detect_ok, value);
	return 0;
}

/* get the maximum current with specified battery */
static int direct_charge_get_battery_max_current(void)
{
	int i;
	int max_cur;
	char *brand = NULL;
	int temp = 25; /* normal temperature is 25c */
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	max_cur = l_di->orig_volt_para_p[0].volt_info[0].cur_th_high;
	brand = hisi_battery_brand();

	for (i = 0; i < l_di->stage_group_size; i++) {
		if (!l_di->orig_volt_para_p[i].bat_info.parse_ok)
			continue;

		if (!strstr(brand, l_di->orig_volt_para_p[i].bat_info.batid))
			continue;

		if ((temp >= l_di->orig_volt_para_p[i].bat_info.temp_high) ||
			(temp < l_di->orig_volt_para_p[i].bat_info.temp_low))
			continue;

		max_cur = l_di->orig_volt_para_p[i].volt_info[0].cur_th_high;
		break;
	}

	hwlog_info("stage_group_size=%d, bat_brand=%s, bat_max_cur=%d\n",
		l_di->stage_group_size, brand, max_cur);

	return max_cur;
}

int direct_charge_check_battery_temp(void)
{
	int bat_temp = 0;
	int bat_temp_cur_max;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	huawei_battery_temp(BAT_TEMP_MIXED, &bat_temp);
	if (bat_temp < DC_LOW_TEMP_MAX + l_di->low_temp_hysteresis) {
		hwlog_info("can not do direct charging again, temp=%d\n",
			bat_temp);
		return -1;
	}
	if (bat_temp >= DC_HIGH_TEMP_MAX - l_di->high_temp_hysteresis) {
		hwlog_info("can not do direct charging again, temp=%d\n",
			bat_temp);
		return -1;
	}

	bat_temp_cur_max = direct_charge_battery_temp_handler(bat_temp);
	if (bat_temp_cur_max == 0) {
		hwlog_info("can not do direct charging, temp=%d\n", bat_temp);
		return -1;
	}

	return 0;
}

int direct_charge_check_battery_voltage(void)
{
	int volt_max = hw_battery_voltage(BAT_ID_MAX);
	int volt_min = hw_battery_voltage(BAT_ID_MIN);
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	if (volt_min < l_di->min_dc_bat_vol ||
		volt_max > l_di->max_dc_bat_vol) {
		hwlog_info("can not do direct charging, v_min=%d v_max=%d\n",
			volt_min, volt_max);
		return -1;
	}

	return 0;
}

bool direct_charge_get_stop_charging_complete_flag(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return true;

	if (l_di->scp_stop_charging_complete_flag) {
		hwlog_info("scp_stop_charging_complete_flag is set\n");
		return true;
	}

	return false;
}

void direct_charge_set_stop_charging_flag(int value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	l_di->stop_charging_flag_error = value;
}

static int direct_charge_get_stop_charging_flag(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	if (l_di->stop_charging_flag_error) {
		hwlog_info("stop_charging_flag_error is set\n");
		return -1;
	}

	return 0;
}

int direct_charge_get_vbus(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return 0;

	return l_di->vadapt;
}

int direct_charge_get_ibus(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return 0;

	return l_di->iadapt;
}

static int direct_charge_get_bat_voltage(int *value)
{
	int btb_vol;
	int package_vol;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di || !l_di->bi_ops)
		return -1;

	if (!l_di->bi_ops->get_bat_btb_voltage)
		return -1;

	if (!l_di->bi_ops->get_bat_package_voltage)
		return -1;

	btb_vol = l_di->bi_ops->get_bat_btb_voltage();
	package_vol = l_di->bi_ops->get_bat_package_voltage();
	if (btb_vol < 0 || package_vol < 0) {
		direct_charge_fill_eh_buf(l_di->dsm_buff,
			sizeof(l_di->dsm_buff),
			DC_EH_GET_VBAT, NULL);
		direct_charge_set_stop_charging_flag(1);
		*value = 0;

		hwlog_err("get battery voltage fail\n");
		return -1;
	}

	*value = (btb_vol < package_vol) ? btb_vol : package_vol;
	return 0;
}

static int direct_charge_get_bat_current(int *value)
{
	s64 temp;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di || !l_di->bi_ops)
		return -1;

	if (!l_di->bi_ops->get_bat_current)
		return -1;

	if (l_di->bi_ops->get_bat_current(value)) {
		direct_charge_fill_eh_buf(l_di->dsm_buff,
			sizeof(l_di->dsm_buff),
			DC_EH_GET_IBAT, NULL);
		direct_charge_set_stop_charging_flag(1);
		*value = 0;

		hwlog_err("get battery current fail\n");
		return -1;
	}

	hwlog_info("cali_b: bat_cur=%d\n", *value);
	if (c_offset_a != 0) {
		temp = (*value * (s64)c_offset_a) + c_offset_b;
		*value = (int)div_s64(temp, 1000000); /* base 1000000 */
		hwlog_info("cali_a: bat_cur=%d, c_offset_a=%ld, c_offset_b=%ld\n",
			*value, c_offset_a, c_offset_b);
	}

	return 0;
}

static int direct_charge_get_bat_sys_voltage(int *value)
{
	/* default one battery */
	if (hw_battery_get_series_num() == 1)
		return direct_charge_get_bat_voltage(value);

	/* get total voltage for multi battery */
	*value = hw_battery_voltage(BAT_ID_ALL);
	return 0;
}

static int direct_charge_get_device_vbus(int *value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di || !l_di->bi_ops)
		return -1;

	if (!l_di->bi_ops->get_vbus_voltage)
		return -1;

	if (l_di->bi_ops->get_vbus_voltage(value)) {
		direct_charge_fill_eh_buf(l_di->dsm_buff,
			sizeof(l_di->dsm_buff),
			DC_EH_GET_DEVICE_VBUS, NULL);
		direct_charge_set_stop_charging_flag(1);
		*value = 0;

		hwlog_err("get lvc or sc device vbus fail\n");
		return -1;
	}

	return 0;
}

static int direct_charge_get_device_ibus(int *value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di || !l_di->bi_ops)
		return -1;

	if (!l_di->bi_ops->get_ls_ibus)
		return -1;

	if (l_di->bi_ops->get_ls_ibus(value)) {
		direct_charge_fill_eh_buf(l_di->dsm_buff,
			sizeof(l_di->dsm_buff),
			DC_EH_GET_DEVICE_IBUS, NULL);
		direct_charge_set_stop_charging_flag(1);
		*value = 0;

		hwlog_err("get lvc or sc device ibus fail\n");
		return -1;
	}

	return 0;
}

static int direct_charge_get_device_temp(int *value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di || !l_di->bi_ops)
		return -1;

	if (!l_di->bi_ops->get_ls_temp)
		return -1;

	if (l_di->bi_ops->get_ls_temp(value)) {
		direct_charge_fill_eh_buf(l_di->dsm_buff,
			sizeof(l_di->dsm_buff),
			DC_EH_GET_DEVICE_TEMP, NULL);
		direct_charge_set_stop_charging_flag(1);
		*value = 0;

		hwlog_err("get lvc or sc device temp fail\n");
		return -1;
	}

	return 0;
}

static int direct_charge_get_device_close_status(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di || !l_di->ls_ops)
		return 1;

	if (!l_di->ls_ops->is_ls_close)
		return 1;

	if (l_di->ls_ops->is_ls_close()) {
		direct_charge_fill_eh_buf(l_di->dsm_buff,
			sizeof(l_di->dsm_buff),
			DC_EH_DEVICE_IS_CLOSE, NULL);
		direct_charge_set_stop_charging_flag(1);

		hwlog_err("lvc or sc device is close\n");
		return 1;
	}

	return 0;
}

static int direct_charge_get_vbus_vbat(int *vbus, int *vbat)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di || !l_di->ls_ops)
		return -1;

	if (l_di->ls_ops->ls_status && l_di->ls_ops->ls_status()) {
		msleep(DC_LS_RECOVERY_DELAY);
		*vbus = get_charger_vbus_vol();
		*vbat = coul_get_battery_voltage_uv();
		*vbat = *vbat / DC_COUL_CURRENT_UNIT_DEVIDE;
		hwlog_err("device error, vbus=%d, vbat=%d\n", *vbus, *vbat);
		return 0;
	}

	direct_charge_get_device_vbus(vbus);
	direct_charge_get_bat_voltage(vbat);

	hwlog_info("device_vbus=%d, vbat=%d\n", *vbus, *vbat);
	return 0;
}

int direct_charge_update_adapter_info(void)
{
	int ret;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	ret = adapter_get_device_info(ADAPTER_PROTOCOL_SCP);
	if (ret) {
		hwlog_err("get adapter info fail\n");
		return -1;
	}
	adapter_show_device_info(ADAPTER_PROTOCOL_SCP);

	ret = adapter_get_chip_vendor_id(ADAPTER_PROTOCOL_SCP,
		&l_di->adaptor_vendor_id);
	if (ret) {
		hwlog_err("get adapter vendor id fail\n");
		return -1;
	}

	return 0;
}

static int direct_charge_get_adapter_voltage(int *value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	if (direct_charge_get_stop_charging_flag())
		return -1;

	if (adapter_get_output_voltage(ADAPTER_PROTOCOL_SCP, value)) {
		direct_charge_fill_eh_buf(l_di->dsm_buff,
			sizeof(l_di->dsm_buff),
			DC_EH_GET_VAPT, NULL);
		direct_charge_set_stop_charging_flag(1);
		*value = 0;

		hwlog_err("get adapter output voltage fail\n");
		return -1;
	}

	return 0;
}

static int direct_charge_get_adapter_current(int *value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	if (direct_charge_get_stop_charging_flag())
		return -1;

	if (l_di->adaptor_vendor_id == ADAPTER_CHIP_IWATT)
		return direct_charge_get_device_ibus(value);

	if (adapter_get_output_current(ADAPTER_PROTOCOL_SCP, value)) {
		direct_charge_fill_eh_buf(l_di->dsm_buff,
			sizeof(l_di->dsm_buff),
			DC_EH_GET_IAPT, NULL);
		direct_charge_set_stop_charging_flag(1);
		*value = 0;

		hwlog_err("get adapter output current fail\n");
		return -1;
	}

	return 0;
}

static int direct_charge_get_adapter_current_set(int *value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	if (direct_charge_get_stop_charging_flag())
		return -1;

	if (adapter_get_output_current_set(ADAPTER_PROTOCOL_SCP, value)) {
		direct_charge_fill_eh_buf(l_di->dsm_buff,
			sizeof(l_di->dsm_buff),
			DC_EH_GET_APT_CURR_SET, NULL);
		direct_charge_set_stop_charging_flag(1);
		*value = 0;

		hwlog_err("get adapter setting current fail\n");
		return -1;
	}

	return 0;
}

static int direct_charge_get_adapter_max_current(int *value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	if (direct_charge_get_stop_charging_flag())
		return -1;

	if (adapter_get_max_current(ADAPTER_PROTOCOL_SCP, value)) {
		direct_charge_fill_eh_buf(l_di->dsm_buff,
			sizeof(l_di->dsm_buff),
			DC_EH_GET_APT_MAX_CURR, NULL);
		direct_charge_set_stop_charging_flag(1);
		*value = 0;

		hwlog_err("get adapter max current fail\n");
		return -1;
	}

	return 0;
}

static int direct_charge_get_adapter_temp(int *value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	if (direct_charge_get_stop_charging_flag())
		return -1;

	if (adapter_get_inside_temp(ADAPTER_PROTOCOL_SCP, value)) {
		direct_charge_fill_eh_buf(l_di->dsm_buff,
			sizeof(l_di->dsm_buff),
			DC_EH_GET_APT_TEMP, NULL);
		direct_charge_set_stop_charging_flag(1);
		*value = 0;

		hwlog_err("get adapter temp fail\n");
		return -1;
	}

	return 0;
}

static int direct_charge_get_adapter_max_current_by_volt(
	struct direct_charge_device *di)
{
	int adapter_type = ADAPTER_TYPE_UNKNOWN;

	adapter_get_adp_type(ADAPTER_PROTOCOL_SCP, &adapter_type);
	switch (adapter_type) {
	case ADAPTER_TYPE_10V2P25A:
		return direct_charge_adapter_volt_handler(di->adaptor_vset,
			di->adp_10v2p25a);
	case ADAPTER_TYPE_10V2P25A_CAR:
		return direct_charge_adapter_volt_handler(di->adaptor_vset,
			di->adp_10v2p25a_car);
	default:
		break;
	}

	return 0;
}

static int direct_charge_set_adapter_voltage(int value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();
	int l_value = value;

	if (!l_di)
		return -1;

	if (direct_charge_get_stop_charging_flag())
		return -1;

	hwlog_info("set adapter_volt=%d,max_volt=%d\n",
		l_value, l_di->max_adapter_vset);

	if (l_value > l_di->max_adapter_vset) {
		l_value = l_di->max_adapter_vset;
		l_di->adaptor_vset = l_di->max_adapter_vset;
	}

	if (adapter_set_output_voltage(ADAPTER_PROTOCOL_SCP, l_value)) {
		direct_charge_fill_eh_buf(l_di->dsm_buff,
			sizeof(l_di->dsm_buff),
			DC_EH_SET_APT_VOLT, NULL);
		direct_charge_set_stop_charging_flag(1);

		hwlog_err("set adapter voltage fail\n");
		return -1;
	}

	return 0;
}

static int direct_charge_set_adapter_voltage_step_by_step(int end)
{
	int vol = STEP_VOL_START;
	int step = 3000; /* step in 3000mv */
	int cnt = 10; /* max times */
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di || !l_di->ls_ops)
		return -1;

	if (!l_di->ls_ops->ls_enable_prepare)
		return direct_charge_set_adapter_voltage(end);

	if (direct_charge_set_adapter_voltage(vol))
		return -1;

	if (l_di->ls_ops->ls_enable_prepare())
		return -1;

	while (cnt--) {
		vol += step;
		hwlog_info("vol step open:%d\n", vol);

		if (vol < end) {
			if (direct_charge_set_adapter_voltage(vol))
				return -1;
			usleep_range(5000, 5200); /* sleep 5ms */
		} else {
			return direct_charge_set_adapter_voltage(end);
		}
	}

	return -1;
}

static int direct_charge_set_adapter_current(int value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();
	int l_value = value;

	if (!l_di)
		return -1;

	if (direct_charge_get_stop_charging_flag())
		return -1;

	hwlog_info("set adapter_cur=%d,max_cur=%d\n",
		l_value, l_di->max_adapter_iset);

	if (l_value > l_di->max_adapter_iset)
		l_value = l_di->max_adapter_iset;

	if (adapter_set_output_current(ADAPTER_PROTOCOL_SCP, l_value)) {
		direct_charge_fill_eh_buf(l_di->dsm_buff,
			sizeof(l_di->dsm_buff),
			DC_EH_SET_APT_CURR, NULL);
		direct_charge_set_stop_charging_flag(1);

		hwlog_err("set adapter current fail\n");
		return -1;
	}

	return 0;
}

int direct_charge_set_adapter_output_enable(int enable)
{
	int ret, i;
	int retry = 3; /* retry 3 times */
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	hwlog_info("set adapter_output_enable=%d\n", enable);

	for (i = 0; i < retry; i++) {
		ret = adapter_set_output_enable(ADAPTER_PROTOCOL_SCP, enable);
		if (!ret)
			break;
	}

	return ret;
}

void direct_charge_set_adapter_default_param(void)
{
	adapter_set_default_param(ADAPTER_PROTOCOL_SCP);
}

static bool direct_charge_get_adapter_antifake_flag(void)
{
	int adapter_type = ADAPTER_TYPE_UNKNOWN;

	adapter_get_adp_type(ADAPTER_PROTOCOL_SCP, &adapter_type);
	switch (adapter_type) {
	/* adapter no need check antifake */
	case ADAPTER_TYPE_10V2A:
	case ADAPTER_TYPE_10V2P25A:
	case ADAPTER_TYPE_10V2P25A_CAR:
		return true;
	default:
		return false;
	}
}

static int direct_charge_get_adapter_gain_current(
	struct direct_charge_device *di)
{
	int adapter_type = ADAPTER_TYPE_UNKNOWN;

	adapter_get_adp_type(ADAPTER_PROTOCOL_SCP, &adapter_type);
	switch (adapter_type) {
	case ADAPTER_TYPE_10V2A:
		return di->gain_curr_10v2a;
	case ADAPTER_TYPE_10V2P25A:
		return di->gain_curr_10v2p25a;
	case ADAPTER_TYPE_10V2P25A_CAR:
		return di->gain_curr_10v2p25a_car;
	default:
		return 0;
	}
}

static int direct_charge_get_device_delta_err(struct direct_charge_device *di)
{
	int adapter_type = ADAPTER_TYPE_UNKNOWN;

	adapter_get_adp_type(ADAPTER_PROTOCOL_SCP, &adapter_type);
	switch (adapter_type) {
	case ADAPTER_TYPE_10V2P25A:
		return di->delta_err_10v2p25a;
	default:
		return di->delta_err;
	}
}

void direct_charge_force_disable_dc_path(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di || !l_di->ls_ops || !l_di->ls_ops->ls_enable)
		return;

	l_di->ls_ops->ls_enable(0);
}

int direct_charge_get_protocol_register_state(void)
{
	if (adapter_get_protocol_register_state(ADAPTER_PROTOCOL_SCP)) {
		hwlog_err("adapter protocol not ready\n");
		return -1;
	}

	direct_charge_set_stage_status(DC_STAGE_ADAPTER_DETECT);
	return 0;
}

static void direct_charge_send_icon_uevent(void)
{
	int adp_limit = 0;
	int cable_limit;
	int bat_limit;
	int max_cur;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	direct_charge_get_adapter_max_current(&adp_limit);
	adp_limit *= l_di->dc_volt_ratio;

	if (pd_dpm_get_ctc_cable_flag())
		cable_limit = l_di->max_current_for_ctc_cable;
	else
		cable_limit = l_di->max_current_for_nonstd_cable;

	bat_limit = direct_charge_get_battery_max_current();
	hwlog_info("adp_limit=%d, cable_limit=%d, bat_limit=%d\n",
		adp_limit, cable_limit, bat_limit);

	if (l_di->cc_cable_detect_enable && l_di->cc_cable_detect_ok == 0)
		max_cur = (bat_limit < cable_limit) ? bat_limit : cable_limit;
	else
		max_cur = bat_limit;

	if (adp_limit)
		max_cur = (max_cur < adp_limit) ? max_cur : adp_limit;

	hwlog_info("cable=%d, ico_cur=%d, max_cur=%d\n",
		l_di->cc_cable_detect_ok, l_di->super_ico_current, max_cur);

	if (max_cur >= l_di->super_ico_current)
		direct_charge_send_super_charging_uevent();
	else
		direct_charge_send_quick_charging_uevent();
}

static void direct_charge_send_soc_decimal_uevent(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	soc_decimal_event_notify(SOC_DECIMAL_NE_DC, &l_di->max_pwr);
}

static void direct_charge_send_cable_type_uevent(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	power_ui_event_notify(POWER_UI_NE_CABLE_TYPE, &l_di->cable_type);
}

static void direct_charge_send_max_power_uevent(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di || l_di->ui_max_pwr <= 0)
		return;

	if (l_di->max_pwr >= l_di->ui_max_pwr)
		power_ui_event_notify(POWER_UI_NE_MAX_POWER, &l_di->ui_max_pwr);
}

static void direct_charge_get_max_power(void)
{
	int adp_limit = 0;
	int cable_limit;
	int bat_limit;
	int max_cur;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	/* get max current by adapter */
	direct_charge_get_adapter_max_current(&adp_limit);
	adp_limit += direct_charge_get_adapter_gain_current(l_di);
	adp_limit *= l_di->dc_volt_ratio;

	/* get max current by cable */
	if (pd_dpm_get_ctc_cable_flag())
		cable_limit = l_di->max_current_for_ctc_cable;
	else
		cable_limit = l_di->max_current_for_nonstd_cable;

	/* get max current by battery */
	bat_limit = direct_charge_get_battery_max_current();

	if (l_di->cc_cable_detect_enable && l_di->cc_cable_detect_ok == 0)
		max_cur = (bat_limit < cable_limit) ? bat_limit : cable_limit;
	else
		max_cur = bat_limit;

	if (adp_limit)
		max_cur = (max_cur < adp_limit) ? max_cur : adp_limit;

	l_di->max_pwr = max_cur * hw_battery_get_series_num() * BAT_RATED_VOLT;
	l_di->max_pwr /= 1000; /* unit: 1000mw */
	hwlog_info("l_adp=%d, l_cable=%d, l_bat=%d, m_cur=%d, m_pwr=%d\n",
		adp_limit, cable_limit, bat_limit, max_cur, l_di->max_pwr);
}

int direct_charge_init_adapter_and_device(void)
{
	struct adapter_init_data aid;
	char tmp_buf[ERR_NO_STRING_SIZE] = { 0 };
	int ret;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di || !l_di->ls_ops || !l_di->bi_ops)
		return -1;

	if (!l_di->ls_ops->ls_init)
		return -1;

	if (!l_di->bi_ops->init)
		return -1;

	aid.scp_mode_enable = 1;
	aid.vset_boundary = l_di->max_adapter_vset;
	aid.iset_boundary = l_di->max_adapter_iset;
	aid.init_voltage = l_di->init_adapter_vset;
	aid.watchdog_timer = 3; /* 3seconds */

	pd_dpm_notify_direct_charge_status(true);

	ret = adapter_set_init_data(ADAPTER_PROTOCOL_SCP, &aid);
	if (ret) {
		snprintf(tmp_buf, sizeof(tmp_buf), "adapter data init fail\n");
		goto fail_init;
	}

	ret = l_di->ls_ops->ls_init();
	if (ret) {
		snprintf(tmp_buf, sizeof(tmp_buf), "device ls init fail\n");
		goto fail_init;
	}

	ret = l_di->bi_ops->init();
	if (ret) {
		snprintf(tmp_buf, sizeof(tmp_buf), "device bi init fail\n");
		goto fail_init;
	}

	hwlog_info("direct charge init succ\n");
	return 0;

fail_init:
	pd_dpm_notify_direct_charge_status(false);

	direct_charge_fill_eh_buf(l_di->dsm_buff, sizeof(l_di->dsm_buff),
		DC_EH_INIT_DIRECT_CHARGE, tmp_buf);

	return -1;
}

static int direct_charge_check_adapter_voltage_accuracy(void)
{
	int vadapt = 0;
	int ibat = 0;
	int vol_err;
	int i;
	int retry = 3; /* retry 3 times */
	char tmp_buf[ERR_NO_STRING_SIZE] = { 0 };
	int error_flag = 0;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	l_di->adaptor_vset = l_di->init_adapter_vset;
	direct_charge_set_adapter_voltage(l_di->adaptor_vset);

	/* delay 500ms */
	usleep_range(500000, 501000);

	/* keep communication with the adaptor within 1 second */
	direct_charge_get_bat_current(&ibat);

	for (i = 0; i < retry; ++i) {
		if (direct_charge_get_adapter_voltage(&vadapt))
			return -1;

		vol_err = vadapt - l_di->adaptor_vset;
		if (vol_err < 0)
			vol_err = -vol_err;

		hwlog_info("[%d]: Verr=%d, Verr_th=%d, Vset=%d, Vread=%d, Vbus=%d\n",
			i, vol_err, l_di->vol_err_th,
			l_di->adaptor_vset, vadapt, get_charger_vbus_vol());

		if (vol_err > l_di->vol_err_th) {
			error_flag = 1;
			break;
		}
	}

	if (error_flag == 0) {
		hwlog_err("adapter voltage accuracy check succ\n");
		return 0;
	}

	/* process error report */
	snprintf(tmp_buf, sizeof(tmp_buf),
		"[%d]: Verr=%d > Verr_th=%d, Vset=%d, Vread=%d, Vbus=%d\n",
		i, vol_err, l_di->vol_err_th,
		l_di->adaptor_vset, vadapt, get_charger_vbus_vol());

	direct_charge_fill_eh_buf(l_di->dsm_buff, sizeof(l_di->dsm_buff),
		DC_EH_APT_VOLTAGE_ACCURACY, tmp_buf);

	power_dsm_dmd_report(POWER_DSM_BATTERY,
		DSM_DIRECT_CHARGE_VOL_ACCURACY, tmp_buf);

	hwlog_err("adapter voltage accuracy check fail\n");
	return -1;
}

static int direct_charge_check_charging_path_leakage_current(void)
{
	int iadapt = 0;
	int ibus = 0;
	int leak_current = 0;
	char tmp_buf[ERR_NO_STRING_SIZE] = { 0 };
	int error_flag = 0;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	switch (l_di->adaptor_vendor_id) {
	case ADAPTER_CHIP_IWATT:
		l_di->adaptor_iset = 400; /* default 400ma */
		if (direct_charge_set_adapter_current(l_di->adaptor_iset))
			return -1;

		/* delay 100ms */
		msleep(100);

		adapter_get_port_leakage_current_flag(ADAPTER_PROTOCOL_SCP,
			&leak_current);
		if (leak_current) {
			error_flag = 1;

			snprintf(tmp_buf, sizeof(tmp_buf),
				"iwatt_adapter usb port current leak, Vbus=%d\n",
			get_charger_vbus_vol());
		}

		break;
	default:
		if (direct_charge_get_device_ibus(&ibus))
			return -1;

		if (direct_charge_get_adapter_current(&iadapt))
			return -1;

		hwlog_info("Iapt=%d, Ileakage_cur_th=%d, Ibus_ls=%d, Vbus=%d\n",
			iadapt, l_di->adaptor_leakage_current_th, ibus,
			get_charger_vbus_vol());

		if (iadapt > l_di->adaptor_leakage_current_th) {
			error_flag = 1;

			snprintf(tmp_buf, sizeof(tmp_buf),
				"Iapt=%d > Ileakage_cur_th=%d, Ibus_ls=%d, Vbus=%d\n",
				iadapt, l_di->adaptor_leakage_current_th, ibus,
				get_charger_vbus_vol());
		}

		break;
	}

	if (error_flag == 0) {
		hwlog_err("usb port leakage current check succ\n");
		return 0;
	}

	/* process error report */
	if (l_di->cc_cable_detect_ok == 1) {
		direct_charge_fill_eh_buf(l_di->dsm_buff,
			sizeof(l_di->dsm_buff),
			DC_EH_USB_PORT_LEAGAGE_CURR, tmp_buf);

		power_dsm_dmd_report(POWER_DSM_BATTERY,
			DSM_DIRECT_CHARGE_USB_PORT_LEAKAGE_CURRENT, tmp_buf);
	}

	hwlog_err("usb port leakage current check fail\n");
	return -1;
}

int direct_charge_open_charging_path(void)
{
	int vbat = 0;
	int ibus = 0;
	int soc;
	int ret;
	int vbat_max = hisi_battery_vbat_max();
	int max_dc_bat_vol;
	int adjust_times = MAX_TIMES_FOR_SET_ADAPTER_VOL;
	char tmp_buf[ERR_NO_STRING_SIZE] = { 0 };
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di || !l_di->ls_ops)
		return -1;

	if (!l_di->ls_ops->ls_enable)
		return -1;

	soc = hisi_battery_capacity();
	direct_charge_get_bat_sys_voltage(&vbat);

	/* first: set voltage and current to adapter */
	l_di->adaptor_vset = vbat * l_di->dc_volt_ratio + l_di->init_delt_vset;
	if (l_di->max_adapter_vset < l_di->adaptor_vset)
		l_di->adaptor_vset = l_di->max_adapter_vset;
	l_di->adaptor_iset = CURRENT_SET_FOR_RES_DETECT;
	direct_charge_set_adapter_voltage_step_by_step(l_di->adaptor_vset);
	direct_charge_set_adapter_current(l_di->adaptor_iset);

	/* delay 50ms */
	msleep(50);

	/* second: enable lvc or sc device */
	ret = l_di->ls_ops->ls_enable(1);
	if (ret) {
		snprintf(tmp_buf, sizeof(tmp_buf), "ls enable fail\n");
		goto fail_open;
	}

	/* delay 10ms */
	usleep_range(10000, 11000);

	if (direct_charge_get_device_ibus(&ibus))
		return -1;

	max_dc_bat_vol = vbat_max > l_di->max_dc_bat_vol ?
		vbat_max : l_di->max_dc_bat_vol;

	hwlog_info("[%d]: Vapt_set=%d, Ibus_ls=%d Vbat_max=%d\n",
		adjust_times, l_di->adaptor_vset, ibus, max_dc_bat_vol);

	/* third: adjust adapter voltage until current to 800ma */
	while (ibus < MIN_CURRENT_FOR_RES_DETECT) {
		direct_charge_get_bat_voltage(&vbat);
		if (max_dc_bat_vol < vbat) {
			snprintf(tmp_buf, sizeof(tmp_buf),
				"vbat too high, Vapt_set=%d\n", vbat);
			goto fail_open;
		}

		adjust_times--;
		if (adjust_times == 0) {
			snprintf(tmp_buf, sizeof(tmp_buf),
				"try too many times, Ibus_ls=%d\n", ibus);
			goto fail_open;
		}

		l_di->adaptor_vset += l_di->vstep;
		direct_charge_set_adapter_voltage(l_di->adaptor_vset);
		/* delay 5ms */
		usleep_range(5000, 5100);
		direct_charge_get_device_ibus(&ibus);

		hwlog_info("[%d]: Vapt_set=%d, Ibus_ls=%d\n",
			adjust_times, l_di->adaptor_vset, ibus);
	}

	hwlog_err("charging path open succ\n");
	return 0;

fail_open:
	direct_charge_fill_eh_buf(l_di->dsm_buff, sizeof(l_di->dsm_buff),
		DC_EH_OPEN_DIRECT_CHARGE_PATH, tmp_buf);
	if (soc >= BATTERY_CAPACITY_HIGH_TH) {
		l_di->dc_open_retry_cnt += 1;
		hwlog_info("current battery capacity is %d, over threshold %d\n",
		soc, BATTERY_CAPACITY_HIGH_TH);
	}

	hwlog_err("charging path open fail\n");
	return -1;
}

int direct_charge_check_full_path_resistance(void)
{
	int vadapt = 0;
	int iadapt = 0;
	int vbus = 0;
	int ibus = 0;
	int rpath;
	int rpath_th;
	int sum = 0;
	int i;
	int retry = 3; /* retry 3 times */
	char tmp_buf[ERR_NO_STRING_SIZE] = { 0 };
	struct direct_charge_device *l_di = direct_charge_get_di();
	int resist_cur_max;

	if (!l_di)
		return -1;

	/* delay 500ms */
	usleep_range(500000, 501000);

	/* keep communication with the adapter within 1 second */
	if (direct_charge_get_adapter_voltage(&vadapt))
		return -1;

	/* keep communication with device within 1 second */
	if (direct_charge_get_device_close_status())
		return -1;

	/* delay 400ms */
	usleep_range(400000, 401000);

	for (i = 0; i < retry; ++i) {
		if (direct_charge_get_device_ibus(&ibus))
			return -1;

		if (direct_charge_get_device_vbus(&vbus))
			return -1;

		if (direct_charge_get_adapter_voltage(&vadapt))
			return -1;

		if (direct_charge_get_adapter_current(&iadapt))
			return -1;

		if (ibus == 0) {
			hwlog_err("ibus is zero\n");
			return -1;
		}

		/* r = v/i, 1000mohm */
		rpath = (vadapt - vbus) * 1000 / ibus;
		sum += rpath;

		hwlog_info("[%d]: Rpath=%d, Vapt=%d, Vbus=%d, Iapt=%d, Ibus=%d\n",
			i, rpath, vadapt, vbus, iadapt, ibus);
	}

	/* calculate average resistance */
	rpath = sum / retry;
	rpath = rpath > 0 ? rpath : -rpath;
	l_di->full_path_resistance = rpath;
	rpath_th = l_di->full_path_res_thld;

	hwlog_info("average Rpath=%d, Rpath_threshold=[%d,%d]\n",
		rpath, -rpath_th, rpath_th);

	if (rpath <= rpath_th) {
		resist_cur_max = direct_charge_resist_handler(l_di, rpath);
		hwlog_err("resist_cur_max=%d\n", resist_cur_max);
		if ((l_di->cc_cable_detect_ok == 0) && (resist_cur_max != 0)) {
			if (pd_dpm_get_ctc_cable_flag())
				l_di->max_current_for_ctc_cable =
					resist_cur_max;
			else
				l_di->max_current_for_nonstd_cable =
					resist_cur_max;
		}
		if (l_di->cc_cable_detect_ok == 0)
			direct_charge_send_icon_uevent();

		hwlog_err("full path resistance check succ\n");
		return 0;
	}

	/* process error report */
	if (l_di->cc_cable_detect_ok == 1) {
		snprintf(tmp_buf, sizeof(tmp_buf),
			"full_res %d is out of[%d, %d]\n",
			rpath, -rpath_th, rpath_th);

		direct_charge_fill_eh_buf(l_di->dsm_buff,
			sizeof(l_di->dsm_buff),
			DC_EH_FULL_PATH_RESISTANCE, tmp_buf);

		power_dsm_dmd_report(POWER_DSM_BATTERY,
			DSM_DIRECT_CHARGE_FULL_PATH_RESISTANCE, tmp_buf);
	}

	hwlog_err("full path resistance check fail\n");
	return -1;
}

int direct_charge_security_check(void)
{
	int ret;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	l_di->dc_succ_flag = DC_ERROR_ADAPTER_VOLTAGE_ACCURACY;
	ret = direct_charge_check_adapter_voltage_accuracy();
	if (ret)
		return -1;

	l_di->dc_succ_flag = DC_ERROR_USB_PORT_LEAKAGE_CURRENT;
	ret = direct_charge_check_charging_path_leakage_current();
	if (ret)
		return -1;

	l_di->dc_succ_flag = DC_ERROR_OPEN_CHARGE_PATH;
	ret = direct_charge_open_charging_path();
	if (ret)
		return -1;

	l_di->dc_succ_flag = DC_ERROR_FULL_REISISTANCE;
	ret = direct_charge_check_full_path_resistance();
	if (ret)
		return -1;

	return 0;
}

static int direct_charge_wait_adapter_antifake(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	/*
	 * if timeout happend, we assume the serivce is dead,
	 * return hash calculate ok anyway
	 */
	if (!wait_for_completion_timeout(&l_di->dc_af_completion,
		DC_AF_WAIT_CT_TIMEOUT)) {
		hwlog_err("bms_auth service wait timeout\n");
		return 0;
	}

	/*
	 * if not timeout,
	 * return the antifake result base on the hash calc result
	 */
	if (l_di->dc_antifake_result == 0) {
		hwlog_err("bms_auth hash calculate fail\n");
		return -1;
	}

	hwlog_info("bms_auth hash calculate ok\n");
	return 0;
}

int direct_charge_check_adapter_antifake(void)
{
	int ret;
	struct direct_charge_device *l_di = direct_charge_get_di();
	struct power_event_notify_data n_data;

	if (!l_di)
		return -1;

	if (!l_di->adp_antifake_enable)
		return 0;

	if (direct_charge_get_adapter_antifake_flag())
		return 0;

	memset(l_di->dc_af_key, 0x00, DC_AF_KEY_LEN);

	ret = adapter_auth_encrypt_start(ADAPTER_PROTOCOL_SCP,
		l_di->adp_antifake_key_index, l_di->dc_af_key, DC_AF_KEY_LEN);
	if (ret)
		goto fail_check;

	l_di->dc_antifake_result = 0;
	n_data.event = "AUTH_DCCT=";
	n_data.event_len = 10; /* length of AUTH_DCCT= */
	power_event_notify(POWER_EVENT_NE_AUTH_DC_SC, &n_data);
	ret = direct_charge_wait_adapter_antifake();

fail_check:
	ret += adapter_auth_encrypt_release(ADAPTER_PROTOCOL_SCP);

	return ret;
}

/* stdandard cable: double 56k cable */
static void direct_charge_detect_std_cable(struct direct_charge_device *di)
{
	int ret;
	bool cc_moisture_status = false;
	int adapter_type = ADAPTER_TYPE_UNKNOWN;
	enum cur_cap c_cap;

	if (di->cc_cable_detect_ok)
		return;

	/* 10v2a and 10v2.25a no need check cable */
	adapter_get_adp_type(ADAPTER_PROTOCOL_SCP, &adapter_type);
	if ((adapter_type == ADAPTER_TYPE_10V2A) ||
		(adapter_type == ADAPTER_TYPE_10V2P25A)) {
		di->cc_cable_detect_ok = 1; /* detect success flag */
		di->full_path_res_thld = di->std_cable_full_path_res_max;
		direct_charge_send_icon_uevent();
		return;
	}

	if (!di->cd_ops || !di->cd_ops->cable_detect) {
		hwlog_err("cd_ops or cable_detect is null\n");
		direct_charge_set_stage_status(DC_STAGE_DEFAULT);
		return;
	}

	ret = di->cd_ops->cable_detect();
	cc_moisture_status = pd_dpm_get_cc_moisture_status();
	c_cap = pd_dpm_get_cvdo_cur_cap();

	if (ret && !cc_moisture_status && (c_cap < PD_DPM_CURR_5A)) {
		hwlog_info("stdandard cable detect fail\n");
		di->cc_cable_detect_ok = 0;
		if (pd_dpm_get_ctc_cable_flag()) {
			di->full_path_res_thld =
				di->ctc_cable_full_path_res_max;
			di->cable_type = DC_STD_CABLE;
			direct_charge_send_icon_uevent();
		} else {
			di->full_path_res_thld =
				di->nonstd_cable_full_path_res_max;
			di->cable_type = DC_NONSTD_CABLE;
			if (di->is_show_ico_first)
				direct_charge_send_icon_uevent();
		}
	} else {
		hwlog_info("stdandard cable detect ok\n");
		di->cable_type = DC_STD_CABLE;
		di->cc_cable_detect_ok = 1;
		di->full_path_res_thld = di->std_cable_full_path_res_max;
		direct_charge_send_icon_uevent();
	}
}

void direct_charge_detect_cable(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	direct_charge_detect_std_cable(l_di);
	direct_charge_set_stage_status(DC_STAGE_SWITCH_DETECT);

	direct_charge_get_max_power();
	direct_charge_send_soc_decimal_uevent();
	direct_charge_send_cable_type_uevent();
	direct_charge_send_max_power_uevent();
}

/* control power supply for adapter protocol communication */
static void direct_charge_adapter_protocol_power_supply(int enable)
{
	int ret;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	if (!l_di->pps_ops || !l_di->pps_ops->power_supply_enable) {
		hwlog_err("pps_ops or power_supply_enable is null\n");
		return;
	}

	ret = l_di->pps_ops->power_supply_enable(enable);
	if (ret) {
		hwlog_err("power supply fail, enable=%d\n", enable);
		return;
	}

	hwlog_info("power supply ok, enable=%d\n", enable);
}

static int direct_charge_open_wired_channel(struct direct_charge_device *di)
{
	if (di->need_wired_sw_off)
		return wired_chsw_set_wired_channel(WIRED_CHANNEL_RESTORE);

	return 0;
}

static int direct_charge_close_wired_channel(struct direct_charge_device *di)
{
	if (di->need_wired_sw_off)
		return wired_chsw_set_wired_channel(WIRED_CHANNEL_CUTOFF);

	return 0;
}

/* switch charging path to normal charging path */
static int direct_charge_switch_to_normal_charging_path(void)
{
	int ret;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	msleep(WAIT_LS_DISCHARGE); /* need to wait device discharge */

	/* no need to check the return value in here */
	hw_usb_ldo_supply_disable(HW_USB_LDO_CTRL_DIRECT_CHARGE);

	ret = direct_charge_open_wired_channel(l_di);

	if (pmic_vbus_irq_is_enabled())
		restore_pluggin_pluggout_interrupt();

	return ret;
}

/* switch charging path to lvc or sc charging path */
static int direct_charge_switch_to_dc_charging_path(void)
{
	int ret;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	if (pmic_vbus_irq_is_enabled()) {
		l_di->cutoff_normal_flag = 1;
		ignore_pluggin_and_pluggout_interrupt();
	}

	hw_usb_ldo_supply_enable(HW_USB_LDO_CTRL_DIRECT_CHARGE);

	if (l_di->scp_work_on_charger) {
		direct_charge_adapter_protocol_power_supply(ENABLE);
		charge_set_hiz_enable(HIZ_MODE_ENABLE);
	}

	msleep(100); /* delay 100ms */
	ret = direct_charge_close_wired_channel(l_di);

	return ret;
}

/* get charging path of direct charge */
static const char *direct_charge_get_charging_path_string(unsigned int path)
{
	if ((path >= PATH_BEGIN) && (path < PATH_END))
		return g_dc_charging_path[path];

	return "illegal charging_path";
}

int direct_charge_switch_charging_path(unsigned int path)
{
	int ret = -1;

	if (path == PATH_NORMAL)
		ret = direct_charge_switch_to_normal_charging_path();

	if ((path == PATH_LVC) || (path == PATH_SC))
		ret = direct_charge_switch_to_dc_charging_path();

	hwlog_info("switch to %d,%s charging path, ret=%d\n",
		path, direct_charge_get_charging_path_string(path), ret);
	return ret;
}

static void direct_charge_enter_specified_mode(
	struct direct_charge_device *di, int mode, int type)
{
	di->adaptor_test_result_type = type;
	di->working_mode = mode;

	direct_charge_set_di(di);
	chg_set_adaptor_test_result(type, DETECT_SUCC);

	if (mode == LVC_MODE) {
		lvc_mode_check();

		if (direct_charge_in_charging_stage() == DC_IN_CHARGING_STAGE)
			blocking_notifier_call_chain(&dc_notify_head,
				LVC_STATUS_CHARGING, NULL);
	} else if (mode == SC_MODE) {
		sc_mode_check();

		if (direct_charge_in_charging_stage() == DC_IN_CHARGING_STAGE)
			blocking_notifier_call_chain(&dc_notify_head,
				SC_STATUS_CHARGING, NULL);
	}
}

static int direct_charge_reset_operate(int type)
{
	int ret;

	switch (type) {
	case DC_RESET_ADAPTER:
		hwlog_info("soft reset adapter\n");
		ret = adapter_soft_reset_slave(ADAPTER_PROTOCOL_SCP);
		break;
	case DC_RESET_MASTER:
		hwlog_info("soft reset master\n");
		ret = adapter_soft_reset_master(ADAPTER_PROTOCOL_SCP);
		msleep(2000); /* sleep 2s */
		break;
	default:
		hwlog_info("reset operate invalid\n");
		ret = -1;
		break;
	}

	return ret;
}

static int direct_charge_detect_adapter(void)
{
	int ret;
	int i;
	int retry = 3; /* retry 3 times */
	int adp_mode = ADAPTER_SUPPORT_UNDEFINED;

	ret = adapter_detect_adapter_support_mode(ADAPTER_PROTOCOL_SCP,
		&adp_mode);
	if (ret == ADAPTER_DETECT_FAIL) {
		for (i = 0; i < retry && ret == ADAPTER_DETECT_FAIL; ++i) {
			/* check if the adapter has been plugged out */
			if (direct_charge_get_stage_status() ==
				DC_STAGE_DEFAULT) {
				hwlog_info("adapter plugged out, stop detect\n");
				return -1;
			}

			/* soft reset adapter */
			if (direct_charge_reset_operate(DC_RESET_ADAPTER)) {
				hwlog_err("soft reset adapter failed\n");
				break;
			}

			ret = adapter_detect_adapter_support_mode(
				ADAPTER_PROTOCOL_SCP,
				&adp_mode);
		}

		if (ret == ADAPTER_DETECT_FAIL) {
			/* check if the adapter has been plugged out */
			if (direct_charge_get_stage_status() ==
				DC_STAGE_DEFAULT) {
				hwlog_info("adapter plugged out, stop detect\n");
				return -1;
			}

			/* soft reset protocol master */
			if (direct_charge_reset_operate(DC_RESET_MASTER))
				hwlog_err("soft reset master failed\n");
			else
				ret = adapter_detect_adapter_support_mode(
					ADAPTER_PROTOCOL_SCP,
					&adp_mode);
		}
	}

	hwlog_info("adapter detect: support_mode=%x ret=%d\n", adp_mode, ret);
	return ret;
}

static bool direct_charge_check_adapter_voltage_range(
	struct direct_charge_device *di)
{
	int max_vadp = 0;
	int min_vadp = 0;
	int bat_num = hw_battery_get_series_num();
	int ratio = di->dc_volt_ratio;
	int delta = di->init_delt_vset;
	int max_vbat = di->max_dc_bat_vol;
	int min_vbat = di->min_dc_bat_vol;
	int max_cfg_volt;
	int min_cfg_volt;
	int ret;

	ret = adapter_get_max_voltage(ADAPTER_PROTOCOL_SCP, &max_vadp);
	if (ret) {
		hwlog_err("get adapter max voltage fail\n");
		return false;
	}

	ret = adapter_get_min_voltage(ADAPTER_PROTOCOL_SCP, &min_vadp);
	if (ret) {
		hwlog_err("get adapter min voltage fail\n");
		return false;
	}

	hwlog_info("max_vadp=%d, max_vbat=%d, bat_num=%d, ratio=%d, delta=%d\n",
		max_vadp, max_vbat, bat_num, ratio, delta);
	hwlog_info("min_vadp=%d, min_vbat=%d, bat_num=%d, ratio=%d, delta=%d\n",
		min_vadp, min_vbat, bat_num, ratio, delta);

	/*
	 * how to calculate max configure voltage as below:
	 * max battery voltage * battery number * ratio + delta
	 */
	max_cfg_volt = max_vbat * bat_num * ratio + delta;
	min_cfg_volt = min_vbat * bat_num * ratio + delta;

	if ((max_vadp > max_cfg_volt) && (min_vadp < min_cfg_volt))
		return true;

	return false;
}

static int direct_charge_redetect_adapter_by_voltage(unsigned int mode)
{
	unsigned int new_mode = UNDEFINED_MODE;
	unsigned int local_mode = direct_charge_get_local_mode();
	struct direct_charge_device *lvc_di = NULL;
	struct direct_charge_device *sc_di = NULL;

	/* we return the raw mode if not b_class adapter */
	if (!(mode & (LVC_MODE | SC_MODE)))
		return mode;

	/* we return the raw mode if local mode not support lvc or sc */
	if (!(local_mode & (LVC_MODE | SC_MODE)))
		return mode;

	lvc_get_di(&lvc_di);
	sc_get_di(&sc_di);

	if (lvc_di && (local_mode & LVC_MODE)) {
		if (direct_charge_check_adapter_voltage_range(lvc_di))
			new_mode |= LVC_MODE;
	}

	if (sc_di && (local_mode & SC_MODE)) {
		if (direct_charge_check_adapter_voltage_range(sc_di))
			new_mode |= SC_MODE;
	}

	hwlog_info("local_mode=%x, adapter_mode=%x, new_adapter_mode=%x\n",
		local_mode, mode, new_mode);

	if (!new_mode)
		direct_charge_set_abnormal_adp_flag(true);

	return new_mode;
}

static unsigned int direct_charge_update_adapter_support_mode(void)
{
	unsigned int adp_mode = ADAPTER_SUPPORT_SCP_B_LVC +
		ADAPTER_SUPPORT_SCP_B_SC;

	adapter_update_adapter_support_mode(ADAPTER_PROTOCOL_SCP, &adp_mode);

	return adp_mode;
}

static unsigned int direct_charge_detect_adapter_support_mode(void)
{
	unsigned int adp_mode = ADAPTER_SUPPORT_UNDEFINED;
	int ret;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return adp_mode;

	l_di->dc_succ_flag = DC_ERROR_ADAPTER_DETECT;

	ret = direct_charge_detect_adapter();
	if (ret) {
		direct_charge_set_stage_status(DC_STAGE_DEFAULT);
		return adp_mode;
	}

	adapter_get_support_mode(ADAPTER_PROTOCOL_SCP, &adp_mode);
	adp_mode &= direct_charge_update_adapter_support_mode();

	if (l_di->adaptor_detect_by_voltage)
		return direct_charge_redetect_adapter_by_voltage(adp_mode);

	return adp_mode;
}

int direct_charge_pre_check(void)
{
	unsigned int local_mode;
	unsigned int adp_mode;

	hwlog_info("pre_check\n");

	local_mode = direct_charge_get_local_mode();
	adp_mode = direct_charge_detect_adapter_support_mode();
	if (adp_mode & local_mode) {
		hwlog_info("adapter support lvc or sc\n");
		return 0;
	}

	hwlog_info("adapter not support lvc or sc\n");
	return -1;
}

static bool direct_charge_check_charge_done(void)
{
	int charge_done_status = get_charge_done_type();
	struct direct_charge_device *lvc_di = NULL;
	struct direct_charge_device *sc_di = NULL;

	lvc_get_di(&lvc_di);
	sc_get_di(&sc_di);

	if (!lvc_di && !sc_di)
		return true;

	if (charge_done_status == CHARGE_DONE) {
		hwlog_info("charge done");
		return true;
	}

	if (lvc_di && (lvc_di->dc_stage == DC_STAGE_CHARGE_DONE)) {
		hwlog_info("lvc charge done");
		return true;
	}

	if (sc_di && (sc_di->dc_stage == DC_STAGE_CHARGE_DONE)) {
		hwlog_info("sc charge done");
		return true;
	}

	return false;
}

static bool direct_charge_check_port_fault(void)
{
	bool cc_vbus_short = false;
	bool cc_moisture_status = false;
	bool intruded_status = false;

	/* cc rp 3.0 can not do high voltage charge */
	cc_vbus_short = pd_dpm_check_cc_vbus_short();
	if (cc_vbus_short)
		hwlog_err("cc match rp3.0, can not do sc charge\n");

	cc_moisture_status = pd_dpm_get_cc_moisture_status();
	if (cc_moisture_status)
		hwlog_err("cc moisture detected\n");

	intruded_status = water_detect_get_intruded_status();
	if (intruded_status)
		hwlog_err("water detect intruded detected\n");

	return !cc_vbus_short && !cc_moisture_status && !intruded_status;
}

void direct_charge_check(void)
{
	unsigned int adp_mode;
	unsigned int local_mode;
	bool cc_safe = false;
	struct direct_charge_device *lvc_di = NULL;
	struct direct_charge_device *sc_di = NULL;

	hwlog_info("check\n");

	lvc_get_di(&lvc_di);
	sc_get_di(&sc_di);

	if (!lvc_di && !sc_di) {
		hwlog_info("local not support direct_charge");
		return;
	}

	if (direct_charge_check_charge_done())
		return;

	local_mode = direct_charge_get_local_mode();

	/* update local mode to switch sc/lvc mode for rt adapter test */
	if (power_cmdline_is_factory_mode())
		local_mode = direct_charge_update_local_mode(local_mode);

	adp_mode = direct_charge_detect_adapter_support_mode();
	hwlog_info("local_mode=%x adapter_mode=%x\n", local_mode, adp_mode);

	if (adp_mode == UNDEFINED_MODE) {
		hwlog_info("undefined adapter mode");
		return;
	}

	cc_safe = direct_charge_check_port_fault();

	if (sc_di &&
		((local_mode & adp_mode) & SC_MODE) &&
		!sc_di->dc_err_report_flag && cc_safe &&
		!sc_di->pri_inversion) {
		sc_di->adapter_type = adp_mode;
		direct_charge_set_can_enter_status(true);
		direct_charge_enter_specified_mode(sc_di, SC_MODE, TYPE_SC);
	} else if (lvc_di &&
		((local_mode & adp_mode) & LVC_MODE) &&
		!lvc_di->dc_err_report_flag &&
		(!lvc_di->cc_protect || cc_safe)) {
		lvc_di->adapter_type = adp_mode;
		direct_charge_set_can_enter_status(true);
		direct_charge_enter_specified_mode(lvc_di, LVC_MODE, TYPE_SCP);
	} else {
		direct_charge_set_can_enter_status(false);
		hwlog_info("neither sc nor lvc matched");
	}
}

int direct_charge_detect_adapter_again(void)
{
	int ret;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	ret = direct_charge_detect_adapter();
	if (ret) {
		direct_charge_set_stage_status(DC_STAGE_DEFAULT);
		return -1;
	}

	chg_set_adaptor_test_result(l_di->adaptor_test_result_type,
		DETECT_SUCC);
	return 0;
}

static void direct_charge_reset_para_in_stop(struct direct_charge_device *di)
{
	if (!di)
		return;

	di->error_cnt = 0;
	di->otp_cnt = 0;
	di->adp_otp_cnt = 0;
	di->reverse_ocp_cnt = 0;
	di->dc_open_retry_cnt = 0;
	di->full_path_resistance = DC_ERROR_RESISTANCE;
	di->adapter_detect_flag = DC_ADAPTER_NOT_DETECT;
	di->dc_succ_flag = DC_ERROR_ADAPTER_DETECT;
	di->quick_charge_flag = 0;
	di->super_charge_flag = 0;
	di->cc_cable_detect_ok = 0;
	di->dc_err_report_flag = FALSE;
	di->sc_conv_ocp_count = 0;
	di->low_temp_hysteresis = 0;
	di->high_temp_hysteresis = 0;
	di->cable_type = DC_UNKNOWN_CABLE;
	di->max_pwr = 0;
	di->pri_inversion = false;
	direct_charge_set_abnormal_adp_flag(false);
	direct_charge_clean_eh_buf(di->dsm_buff, sizeof(di->dsm_buff));
}

static void direct_charge_reset_para_in_exit(struct direct_charge_device *di)
{
	if (!di)
		return;

	di->full_path_resistance = DC_ERROR_RESISTANCE;
	di->adapter_detect_flag = DC_ADAPTER_NOT_DETECT;
	di->dc_succ_flag = DC_ERROR_ADAPTER_DETECT;
	direct_charge_set_stop_charging_flag(0);
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
	di->cc_cable_detect_ok = 0;
	di->ibat_abnormal_cnt = 0;
	di->max_adaptor_cur = 0;
	di->dc_open_retry_cnt = 0;
	di->otp_cnt = 0;
	di->adp_otp_cnt = 0;
	di->reverse_ocp_cnt = 0;
	di->dc_err_report_flag = FALSE;
	di->low_temp_hysteresis = 0;
	di->high_temp_hysteresis = 0;
	di->cable_type = DC_UNKNOWN_CABLE;
	di->max_pwr = 0;
	di->bat_temp_err_flag = false;
	di->rt_test_succ = false;
	di->pri_inversion = false;
	direct_charge_set_abnormal_adp_flag(false);
	direct_charge_clean_eh_buf(di->dsm_buff, sizeof(di->dsm_buff));
}

void direct_charge_exit(void)
{
	struct direct_charge_device *lvc_di = NULL;
	struct direct_charge_device *sc_di = NULL;

	hwlog_info("exit\n");

	lvc_get_di(&lvc_di);
	sc_get_di(&sc_di);

	if (!lvc_di && !sc_di) {
		hwlog_info("local not support direct_charge\n");
		return;
	}

	if (!power_cmdline_is_factory_mode())
		direct_charge_set_disable_flags(DC_CLEAR_DISABLE_FLAGS,
			DC_DISABLE_SYS_NODE);

	direct_charge_reset_para_in_exit(lvc_di);
	direct_charge_reset_para_in_exit(sc_di);
}

int direct_charge_get_cutoff_normal_flag(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return 0;

	return l_di->cutoff_normal_flag;
}

void direct_charge_update_cutoff_normal_flag(void)
{
	struct direct_charge_device *lvc_di = NULL;
	struct direct_charge_device *sc_di = NULL;
	int cutoff_normal_flag = 0;

	hwlog_info("update_cutoff_normal_flag\n");

	lvc_get_di(&lvc_di);
	sc_get_di(&sc_di);

	if (lvc_di)
		cutoff_normal_flag += lvc_di->cutoff_normal_flag;

	if (sc_di)
		cutoff_normal_flag += sc_di->cutoff_normal_flag;

	if (cutoff_normal_flag && pmic_vbus_irq_is_enabled()) {
		if (lvc_di) {
			if (lvc_di->cutoff_normal_flag) {
				lvc_di->cutoff_normal_flag = 0;
				hwlog_info("clear lvc cutoff_normal_flag\n");
			}
		}

		if (sc_di) {
			if (sc_di->cutoff_normal_flag) {
				sc_di->cutoff_normal_flag = 0;
				hwlog_info("clear sc cutoff_normal_flag\n");
			}
		}
	} else {
		if (lvc_di)
			direct_charge_reset_para_in_stop(lvc_di);

		if (sc_di)
			direct_charge_reset_para_in_stop(sc_di);
	}
}

static void direct_charge_select_charging_volt_param(void)
{
	int i;
	char *brand = NULL;
	int tbat = 0;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

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

void direct_charge_select_charging_param(void)
{
	int cur_th_high;
	int vbat_th;
	int vol_th;
	int tbat = 0;
	int tbat_max_cur;
	int path_resist;
	int path_resist_max_cur;
	int adp_max_cur = 0;
	int adp_th;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	/* get max current with specified temp */
	huawei_battery_temp(BAT_TEMP_MIXED, &tbat);
	tbat_max_cur = direct_charge_battery_temp_handler(tbat);
	if (tbat_max_cur == 0) {
		hwlog_info("temp=%d, can not do direct charging\n", tbat);
		if (tbat < DC_LOW_TEMP_MAX)
			l_di->low_temp_hysteresis = l_di->orig_low_temp_hysteresis;
		if (tbat >= DC_HIGH_TEMP_MAX)
			l_di->high_temp_hysteresis = l_di->orig_high_temp_hysteresis;
		l_di->scp_stop_charging_flag_info = 1;
		l_di->bat_temp_err_flag = true;
	}

	/* get max current with specified resist */
	path_resist = l_di->full_path_resistance;
	path_resist = path_resist >= 0 ? path_resist : -path_resist;
	path_resist_max_cur = direct_charge_resist_handler(l_di, path_resist);

	/* get max current with adapter */
	if (direct_charge_get_adapter_max_current(&adp_max_cur))
		return;

	adp_max_cur += direct_charge_get_adapter_gain_current(l_di);
	if (adp_max_cur != l_di->max_adaptor_cur) {
		hwlog_info("adp_max_cur=%d\n", adp_max_cur);
		l_di->max_adaptor_cur = adp_max_cur;
	}

	if (l_di->use_5a) {
		if (adp_max_cur == 4500) /* 4500ma */
			adp_max_cur = l_di->volt_para[0].cur_th_high;
	}

	if (l_di->use_8a) {
		if (adp_max_cur == 4000) /* 4000ma */
			adp_max_cur = l_di->volt_para[0].cur_th_high;
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

	if (l_di->cc_cable_detect_enable) {
		if (l_di->cc_cable_detect_ok == 0) {
			if (pd_dpm_get_ctc_cable_flag())
				cur_th_high = (cur_th_high >
					l_di->max_current_for_ctc_cable) ?
					l_di->max_current_for_ctc_cable :
					cur_th_high;
			else
				cur_th_high = (cur_th_high >
					l_di->max_current_for_nonstd_cable) ?
					l_di->max_current_for_nonstd_cable :
					cur_th_high;
		}

		hwlog_info("cc_cable=%d, cur_th_high=%d\n",
			l_di->cc_cable_detect_ok, cur_th_high);
	}

	cur_th_high = cur_th_high > tbat_max_cur ? tbat_max_cur : cur_th_high;

	adp_th = direct_charge_get_adapter_max_current_by_volt(l_di);
	if (adp_th)
		cur_th_high = (cur_th_high > l_di->dc_volt_ratio * adp_th) ?
			(l_di->dc_volt_ratio * adp_th) : cur_th_high;

	if (path_resist_max_cur != 0)
		cur_th_high = cur_th_high > path_resist_max_cur ?
			path_resist_max_cur : cur_th_high;

	cur_th_high = cur_th_high > l_di->dc_volt_ratio * adp_max_cur ?
		l_di->dc_volt_ratio * adp_max_cur : cur_th_high;
	l_di->cur_ibat_th_high = cur_th_high > l_di->sysfs_iin_thermal ?
		l_di->sysfs_iin_thermal : cur_th_high;
	/* cur_stage include cc and cv stage so divide 2 */
	l_di->cur_ibat_th_low = l_di->volt_para[l_di->cur_stage / 2].cur_th_low;
}

static int direct_charge_jump_stage_if_need(int cur_stage)
{
	int i;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	/* 2: cc & cv */
	for (i = 0; i < 2 * l_di->stage_size; ++i) {
		if (l_di->stage_need_to_jump[i] == -1)
			return cur_stage;

		if (cur_stage == l_di->stage_need_to_jump[i]) {
			hwlog_info("jump stage to %d\n", cur_stage);
			return direct_charge_jump_stage_if_need(cur_stage + 1);
		}
	}

	return cur_stage;
}

static void direct_charge_select_charging_stage(void)
{
	int i;
	int vbat_th;
	int cur_stage = 0;
	int stage_size;
	int vbat = 0;
	int ibat = 0;
	int iadp = 0;
	int max_cnt = 10; /* 10 counts */
	char tmp_buf[ERR_NO_STRING_SIZE] = { 0 };
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	direct_charge_get_bat_voltage(&vbat);
	direct_charge_get_bat_current(&ibat);
	direct_charge_get_adapter_current(&iadp);

	stage_size = l_di->stage_size;
	l_di->vbat = vbat;
	l_di->ibat = ibat;

	if (iadp < l_di->ibat_abnormal_th)
		l_di->ibat_abnormal_cnt++;
	else
		l_di->ibat_abnormal_cnt = 0;

	if (l_di->ibat_abnormal_cnt > max_cnt) {
		direct_charge_set_stop_charging_flag(1);
		snprintf(tmp_buf, sizeof(tmp_buf),
			"ibat abnormal, stop direct charge\n");
		direct_charge_fill_eh_buf(l_di->dsm_buff,
			sizeof(l_di->dsm_buff), DC_EH_IBAT_ABNORMAL, tmp_buf);
		return;
	}

	l_di->pre_stage = l_di->cur_stage;
	for (i = stage_size - 1; i >= 0; --i) {
		vbat_th = l_di->volt_para[i].vol_th + l_di->compensate_v;
		vbat_th = (vbat_th > l_di->volt_para[stage_size - 1].vol_th) ?
			l_di->volt_para[stage_size - 1].vol_th : vbat_th;
		/* 0: cc stage, 1: cv stage, 2: max stage */
		if (vbat >= vbat_th && ibat <= l_di->volt_para[i].cur_th_low) {
			cur_stage = 2 * i + 2;
			break;
		} else if (vbat >= vbat_th) {
			cur_stage = 2 * i + 1;
			break;
		}
	}

	if (i < 0)
		cur_stage = 0;

	if (cur_stage < l_di->pre_stage)
		cur_stage = l_di->pre_stage;

	if (l_di->first_cc_stage_timer_in_min) {
		/* 0: cc stage, 1: cv stage, 2: max stage */
		if (cur_stage == 0 || cur_stage == 1) {
			if (time_after(jiffies, l_di->first_cc_stage_timeout)) {
				hwlog_info("first_cc_stage in %d min, set stage=2\n",
					l_di->first_cc_stage_timer_in_min);
				cur_stage = 2;
			}
		}
	}

	if (cur_stage != l_di->cur_stage)
		l_di->cur_stage = direct_charge_jump_stage_if_need(cur_stage);
}

static void direct_charge_soh_policy(struct direct_charge_device *di)
{
	int i;
	int volt_max;
	int cur_max;
	bool print = false;

	if (!di)
		return;

	if ((di->stage_size < 1) || (di->stage_size > DC_VOLT_LEVEL)) {
		hwlog_err("stage_size %d out of range\n", di->stage_size);
		return;
	}

	volt_max = di->orig_volt_para[di->stage_size - 1].vol_th -
		di->vterm_dec;
	if (di->volt_para[di->stage_size - 1].vol_th != volt_max) {
		di->volt_para[di->stage_size - 1].vol_th = volt_max;
		print = true;
	}

	for (i = 0; i < di->stage_size - 1; i++)
		di->volt_para[i].vol_th =
			(di->orig_volt_para[i].vol_th < volt_max) ?
			di->orig_volt_para[i].vol_th : volt_max;

	if (di->ichg_ratio == 0)
		di->ichg_ratio = BASP_PARA_SCALE;
	cur_max = di->orig_volt_para[0].cur_th_high *
		di->ichg_ratio / BASP_PARA_SCALE;
	if (di->volt_para[0].cur_th_high != cur_max) {
		di->volt_para[0].cur_th_high = cur_max;
		print = true;
	}
	di->volt_para[0].cur_th_low =
		di->orig_volt_para[0].cur_th_low;

	for (i = 1; i < di->stage_size; i++) {
		di->volt_para[i].cur_th_high =
			(di->orig_volt_para[i].cur_th_high <= cur_max) ?
			di->orig_volt_para[i].cur_th_high : cur_max;
		di->volt_para[i].cur_th_low =
			di->orig_volt_para[i].cur_th_low;
	}

	if (!print)
		return;

	hwlog_info("dc_volt_dec:%d, dc_cur_ratio:%d\n",
		di->vterm_dec, di->ichg_ratio);
	for (i = 0; i < di->stage_size; i++)
		hwlog_info("volt_para[%d]: vol_th:%d cur_th_high:%d cur_th_low:%d\n",
			i, di->volt_para[i].vol_th,
			di->volt_para[i].cur_th_high,
			di->volt_para[i].cur_th_low);
}

static void direct_charge_basp_policy(struct direct_charge_device *di,
	bool force_update)
{
	int ret;
	int i;
	int cur_level;
	AGING_SAFE_POLICY_TYPE basp = { 0 };

	if (!di)
		return;

	ret = hisi_battery_aging_safe_policy(&basp);
	if (ret) {
		direct_charge_soh_policy(di);
		return;
	}

	cur_level = basp.level;

	if (cur_level == di->last_basp_level && force_update == false)
		return;

	if ((di->stage_size < 1) || (di->stage_size > DC_VOLT_LEVEL)) {
		hwlog_err("stage_size %d out of range\n", di->stage_size);
		return;
	}

	di->volt_para[di->stage_size - 1].vol_th =
		di->orig_volt_para[di->stage_size - 1].vol_th -
		basp.dc_volt_dec;
	for (i = 0; i < di->stage_size - 1; i++)
		di->volt_para[i].vol_th =
			(di->orig_volt_para[i].vol_th <
			di->volt_para[di->stage_size - 1].vol_th) ?
			di->orig_volt_para[i].vol_th :
			di->volt_para[di->stage_size - 1].vol_th;

	switch (basp.cur_ratio_policy) {
	case BASP_RATIO_POLICY_ALL:
		di->volt_para[di->stage_size - 1].cur_th_high =
			di->orig_volt_para[di->stage_size - 1].cur_th_high *
			basp.cur_ratio / BASP_PARA_SCALE;
		di->volt_para[di->stage_size - 1].cur_th_low =
			di->orig_volt_para[di->stage_size - 1].cur_th_low;

		for (i = 0; i < di->stage_size - 1; i++) {
			di->volt_para[i].cur_th_high =
				di->orig_volt_para[i].cur_th_high *
				basp.cur_ratio / BASP_PARA_SCALE;
			di->volt_para[i].cur_th_low =
				di->orig_volt_para[i].cur_th_low *
				basp.cur_ratio / BASP_PARA_SCALE;
		}
		break;
	case BASP_RATIO_POLICY_MAX:
		di->volt_para[0].cur_th_high =
			di->orig_volt_para[0].cur_th_high *
			basp.cur_ratio / BASP_PARA_SCALE;
		di->volt_para[0].cur_th_low =
			di->orig_volt_para[0].cur_th_low;

		for (i = 1; i < di->stage_size; i++) {
			di->volt_para[i].cur_th_high =
				(di->orig_volt_para[i].cur_th_high <=
				di->volt_para[0].cur_th_high) ?
				di->orig_volt_para[i].cur_th_high :
				di->volt_para[0].cur_th_high;
			di->volt_para[i].cur_th_low =
				di->orig_volt_para[i].cur_th_low;
		}
		break;
	default:
		break;
	}

	di->last_basp_level = cur_level;
	hwlog_info("cur_level=%d\n", cur_level);

	for (i = 0; i < di->stage_size; i++)
		hwlog_info("volt_para[%d]: vol_th:%d cur_th_high:%d cur_th_low:%d\n",
		i,
		di->volt_para[i].vol_th,
		di->volt_para[i].cur_th_high,
		di->volt_para[i].cur_th_low);
}

static void direct_charge_regulation(void)
{
	int ret;
	int iadp = 0;
	int iadp_set = 0;
	int vbat;
	int ibat;
	int vbat_th;
	int ibat_th_high;
	int ibat_th_low;
	int ratio;
	int delta_err;
	char tmp_buf[ERR_NO_STRING_SIZE] = { 0 };
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	direct_charge_get_adapter_current(&iadp);
	direct_charge_get_adapter_current_set(&iadp_set);
	direct_charge_get_adapter_voltage(&l_di->vadapt);
	direct_charge_get_adapter_temp(&l_di->tadapt);
	direct_charge_get_device_ibus(&l_di->ls_ibus);
	direct_charge_get_device_vbus(&l_di->ls_vbus);
	direct_charge_get_device_temp(&l_di->tls);

	delta_err = direct_charge_get_device_delta_err(l_di);
	l_di->iadapt = iadp;
	vbat = l_di->vbat;
	ibat = l_di->ibat;
	vbat_th = l_di->cur_vbat_th;
	ibat_th_high = l_di->cur_ibat_th_high;
	ibat_th_low = l_di->cur_ibat_th_low;

	hwlog_info("cur_stage[%d]: vbat=%d vbat_th=%d\t"
		"ibat=%d ibat_th_high=%d ibat_th_low=%d\t"
		"vadp=%d iadp=%d iadp_set=%d\t"
		"ls_vbus=%d ls_ibus=%d tadp=%d tls=%d\n",
		l_di->cur_stage, vbat, vbat_th,
		ibat, ibat_th_high, ibat_th_low,
		l_di->vadapt, iadp, iadp_set,
		l_di->ls_vbus, l_di->ls_ibus, l_di->tadapt, l_di->tls);

	/* record rt adapter test result when test succ */
	if (power_cmdline_is_factory_mode()) {
		if (ibat >= (int)l_di->rt_curr_th)
			l_di->rt_test_succ = true;
	}

	/* check temperature with device */
	if (l_di->tls > l_di->max_tls) {
		snprintf(tmp_buf, sizeof(tmp_buf),
			"tls=%d > %d, stop direct charge\n",
			l_di->tls, l_di->max_tls);
		direct_charge_fill_eh_buf(l_di->dsm_buff,
			sizeof(l_di->dsm_buff), DC_EH_TLS_ABNORMAL, tmp_buf);
		direct_charge_set_stop_charging_flag(1);
		return;
	}

	/* check temperature with adapter */
	if (l_di->tadapt > l_di->max_tadapt) {
		snprintf(tmp_buf, sizeof(tmp_buf),
			"tadp=%d > %d, stop direct charge\n",
			l_di->tadapt, l_di->max_tadapt);
		direct_charge_fill_eh_buf(l_di->dsm_buff,
			sizeof(l_di->dsm_buff), DC_EH_TADP_ABNORMAL, tmp_buf);

		l_di->adp_otp_cnt++;
		if (l_di->adp_otp_cnt >= ADP_OTP_CNT) {
			l_di->adp_otp_cnt = ADP_OTP_CNT;
			power_dsm_dmd_report(POWER_DSM_BATTERY,
				DSM_DIRECT_CHARGE_ADAPTER_OTP, tmp_buf);
		}

		direct_charge_set_stop_charging_flag(1);
		return;
	}

	/* keep communication with device within 1 second */
	ret = direct_charge_get_device_close_status();
	if (ret)
		return;

	ratio = l_di->dc_volt_ratio;
	if (ratio == 0) {
		hwlog_err("do nothing, ratio is 0\n");
		return;
	}

	/* 2: cc and cv stage */
	if (l_di->cur_stage % 2) {
		if (vbat > vbat_th) {
			l_di->adaptor_vset += ratio * (vbat_th - vbat);
			direct_charge_set_adapter_voltage(l_di->adaptor_vset);
			return;
		}

		if (iadp > ibat_th_high / ratio) {
			l_di->adaptor_vset -= l_di->vstep;
			direct_charge_set_adapter_voltage(l_di->adaptor_vset);
			return;
		}

		if (ibat > ibat_th_high) {
			l_di->adaptor_vset -= l_di->vstep;
			direct_charge_set_adapter_voltage(l_di->adaptor_vset);
			return;
		}

		if (ibat > ibat_th_high - delta_err) {
			hwlog_info("do nothing\n");
			return;
		}

		/* adjust 1000ma/ratio per step */
		if (l_di->adaptor_iset < (ibat_th_high - 1000) / ratio) {
			l_di->adaptor_iset += 1000 / ratio;
			direct_charge_set_adapter_current(l_di->adaptor_iset);
			return;
		} else if (l_di->adaptor_iset < ibat_th_high / ratio) {
			l_di->adaptor_iset = ibat_th_high / ratio;
			direct_charge_set_adapter_current(l_di->adaptor_iset);
			return;
		} else if (iadp < (ibat_th_high - delta_err) / ratio) {
			l_di->adaptor_vset += l_di->vstep;
			direct_charge_set_adapter_voltage(l_di->adaptor_vset);
			return;
		}
	} else {
		if (iadp > ibat_th_high / ratio) {
			l_di->adaptor_vset -= l_di->vstep;
			direct_charge_set_adapter_voltage(l_di->adaptor_vset);
			return;
		}

		if (ibat > ibat_th_high) {
			l_di->adaptor_vset -= l_di->vstep;
			direct_charge_set_adapter_voltage(l_di->adaptor_vset);
			return;
		}

		if (ibat > ibat_th_high - delta_err) {
			hwlog_info("do nothing\n");
			return;
		}

		/* adjust 1000ma/ratio per step */
		if (l_di->adaptor_iset < (ibat_th_high - 1000) / ratio) {
			l_di->adaptor_iset += 1000 / ratio;
			direct_charge_set_adapter_current(l_di->adaptor_iset);
			return;
		} else if (l_di->adaptor_iset < ibat_th_high / ratio) {
			l_di->adaptor_iset = ibat_th_high / ratio;
			direct_charge_set_adapter_current(l_di->adaptor_iset);
			return;
		} else if (iadp < (ibat_th_high - delta_err) / ratio) {
			l_di->adaptor_vset += l_di->vstep;
			direct_charge_set_adapter_voltage(l_di->adaptor_vset);
			return;
		}
	}
}

static bool direct_charge_check_priority_inversion(struct direct_charge_device *di)
{
	int r_cur;
	struct direct_charge_device *other = NULL;

	if (!di)
		return false;

	if (di->working_mode == SC_MODE) {
		lvc_get_di(&other);
		if (!other)
			return false;
		if (di->sysfs_iin_thermal == di->iin_thermal_default || di->sysfs_iin_thermal == 0)
			return false;
		if (di->sysfs_iin_thermal > di->cur_inversion || di->sysfs_iin_thermal > di->cur_ibat_th_high)
			return false;
		if (other->sysfs_iin_thermal && di->sysfs_iin_thermal >= other->sysfs_iin_thermal)
			return false;
		if (di->max_adaptor_cur && di->sysfs_iin_thermal > di->max_adaptor_cur)
			return false;
		if (!di->cc_cable_detect_ok && di->sysfs_iin_thermal > other->max_current_for_nonstd_cable)
			return false;
		r_cur = direct_charge_resist_handler(other, abs(other->full_path_resistance));
		if (r_cur && di->sysfs_iin_thermal > r_cur)
			return false;
		if (((direct_charge_get_local_mode() & di->adapter_type) &
			LVC_MODE) && !other->dc_err_report_flag) {
			hwlog_info("trigger inversion, sc switch to lvc\n");
			di->pri_inversion = true;
			other->pri_inversion = false;
			return true;
		}
	} else if (di->working_mode == LVC_MODE) {
		sc_get_di(&other);
		if (!other)
			return false;
		if (!other->pri_inversion)
			return false;
		if (other->sysfs_iin_thermal <= other->cur_inversion)
			return false;
		if (di->cur_ibat_th_high >= other->sysfs_iin_thermal)
			return false;
		if (!other->dc_err_report_flag) {
			hwlog_info("trigger inversion, lvc switch to sc\n");
			di->pri_inversion = true;
			other->pri_inversion = false;
			return true;
		}
	}
	return false;
}

void direct_charge_control_work(struct work_struct *work)
{
	struct direct_charge_device *di = NULL;
	int t;

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
		hwlog_info("direct charge stop\n");
		direct_charge_stop_charging();
		return;
	}

	/* check if sc switch to lvc or lvc switch to sc */
	if (direct_charge_check_priority_inversion(di)) {
		direct_charge_stop_charging();
		return;
	}

	if (DOUBLE_SIZE * di->stage_size == di->cur_stage) {
		hwlog_info("cur_stage=%d, vbat=%d, ibat=%d\n",
			di->cur_stage, di->vbat, di->ibat);
		hwlog_info("direct charge done\n");
		direct_charge_set_stage_status(DC_STAGE_CHARGE_DONE);
		direct_charge_stop_charging();
		return;
	}

	direct_charge_regulation();

	t = di->charge_control_interval;
	hrtimer_start(&di->control_timer,
		ktime_set(t / MSEC_PER_SEC, (t % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);
}

void direct_charge_calc_thld_work(struct work_struct *work)
{
	struct direct_charge_device *di = NULL;
	int t;

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
		hwlog_info("direct charge stop, stop calc threshold\n");
		return;
	}

	if (di->pri_inversion)
		return;

	direct_charge_basp_policy(di, false);
	direct_charge_select_charging_stage();
	direct_charge_select_charging_param();

	if (DOUBLE_SIZE * di->stage_size == di->cur_stage) {
		hwlog_info("direct charge done, stop calc threshold\n");
		return;
	}

	t = di->threshold_caculation_interval;
	hrtimer_start(&di->calc_thld_timer,
		ktime_set(t / MSEC_PER_SEC, (t % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);
}

void direct_charge_kick_wtd_work(struct work_struct *work)
{
	struct direct_charge_device *di = NULL;
	int t;
	int ibat = 0;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct direct_charge_device, kick_wtd_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	if (di->can_stop_kick_wdt) {
		if (di->stop_charging_flag_error ||
			di->scp_stop_charging_flag_info ||
			(di->sysfs_enable_charger == 0)) {
			hwlog_info("direct charge stop, stop kick watchdog\n");
			return;
		}

		if (DOUBLE_SIZE * di->stage_size == di->cur_stage) {
			hwlog_info("direct charge done, stop kick watchdog\n");
			return;
		}
	}

	if (di->ls_ops->kick_watchdog)
		di->ls_ops->kick_watchdog();
	else
		direct_charge_get_bat_current(&ibat);

	t = KICK_WATCHDOG_TIME;
	hrtimer_start(&di->kick_wtd_timer,
		ktime_set(t / MSEC_PER_SEC, (t % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);
}

enum hrtimer_restart direct_charge_calc_thld_timer_func(struct hrtimer *timer)
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

enum hrtimer_restart direct_charge_control_timer_func(struct hrtimer *timer)
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

enum hrtimer_restart direct_charge_kick_wtd_timer_func(struct hrtimer *timer)
{
	struct direct_charge_device *di = NULL;

	if (!timer) {
		hwlog_err("timer is null\n");
		return HRTIMER_NORESTART;
	}

	di = container_of(timer, struct direct_charge_device, kick_wtd_timer);
	if (!di) {
		hwlog_err("di is null\n");
		return HRTIMER_NORESTART;
	}

	queue_work(di->kick_wtd_wq, &di->kick_wtd_work);

	return HRTIMER_NORESTART;
}

int direct_charge_fault_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct direct_charge_device *di = NULL;
	unsigned int stage = direct_charge_get_stage_status();

	if (!nb) {
		hwlog_err("nb is null\n");
		return NOTIFY_OK;
	}

	di = container_of(nb, struct direct_charge_device, fault_nb);
	if (!di) {
		hwlog_err("di is null\n");
		return NOTIFY_OK;
	}

	if (stage < DC_STAGE_SECURITY_CHECK || stage == DC_STAGE_CHARGE_DONE) {
		hwlog_err("ignore notifier when not in direct charging\n");
		return NOTIFY_OK;
	}

	di->charge_fault = (enum direct_charge_fault_type)event;
	di->fault_data = (struct nty_data *)data;
	schedule_work(&di->fault_work);

	return NOTIFY_OK;
}

void direct_charge_start_charging(void)
{
	int t;
	int msecs;
	struct direct_charge_device *l_di = direct_charge_get_di();

	hwlog_info("start_charging\n");

	if (!l_di || !l_di->ls_ops)
		return;

	if (!l_di->ls_ops->watchdog_config_ms)
		return;

	/* first: send charging event */
	charge_send_uevent(VCHRG_START_AC_CHARGING_EVENT);

	/* second: request wake lock */
	direct_charge_wake_lock();

	/* third: calculate first cc stage timeout */
	hwlog_info("first_cc_stage time=%d\n",
		l_di->first_cc_stage_timer_in_min);
	if (l_di->first_cc_stage_timer_in_min) {
		/* 8a maximum 5 min, ap will not suspend, so use jiffies */
		msecs = l_di->first_cc_stage_timer_in_min * 60 * MSEC_PER_SEC;
		l_di->first_cc_stage_timeout = jiffies +
			msecs_to_jiffies(msecs);
	}

	/* fourth: select charging voltage parameter */
	direct_charge_select_charging_volt_param();

	/* fifth: select charging stage */
	direct_charge_select_charging_stage();

	/* sixth: execute basp policy */
	direct_charge_basp_policy(l_di, true);

	if (l_di->cur_stage == 0) {
		/* 500: unit is 500ma */
		l_di->volt_para[0].cur_th_high -= l_di->sc_conv_ocp_count * 500;
		hwlog_info("sc_conv_ocp_count=%d, cur_th_high[0]=%d\n",
			l_di->sc_conv_ocp_count,
			l_di->volt_para[0].cur_th_high);
	}

	chg_set_adaptor_test_result(l_di->adaptor_test_result_type,
		PROTOCOL_FINISH_SUCC);
	direct_charge_set_stage_status(DC_STAGE_CHARGING);

	/* seventh: select charging parameter */
	direct_charge_select_charging_param();

	/* eighth: config watchdog */
	l_di->ls_ops->watchdog_config_ms(WATCHDOG_TIMEOUT);
	l_di->can_stop_kick_wdt = 0;

	/* ninth: start charging control timer */
	t = l_di->charge_control_interval;
	hrtimer_start(&l_di->control_timer,
		ktime_set(t / MSEC_PER_SEC, (t % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);

	/* tenth: start threshold calculation timer */
	t = l_di->threshold_caculation_interval;
	hrtimer_start(&l_di->calc_thld_timer,
		ktime_set(t / MSEC_PER_SEC, (t % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);

	/* eleventh: start kick watchdog timer */
	t = KICK_WATCHDOG_TIME;
	hrtimer_start(&l_di->kick_wtd_timer,
		ktime_set(t / MSEC_PER_SEC, (t % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);
}

void direct_charge_stop_charging(void)
{
	int ret;
	int vbus = 0;
	int vbat = 0;
	int vadp = 0;
	struct direct_charge_device *lvc_di = NULL;
	struct direct_charge_device *sc_di = NULL;
	struct direct_charge_device *l_di = direct_charge_get_di();

	hwlog_info("stop_charging\n");

	if (!l_di || !l_di->ls_ops || !l_di->bi_ops)
		return;

	lvc_get_di(&lvc_di);
	sc_get_di(&sc_di);

	if (!lvc_di && !sc_di) {
		hwlog_info("local not support direct_charge");
		return;
	}

	if (direct_charge_get_stage_status() != DC_STAGE_CHARGE_DONE)
		l_di->adapter_detect_flag = DC_ADAPTER_NOT_DETECT;

	if (l_di->stop_charging_flag_error)
		l_di->error_cnt += 1;

	if (l_di->stop_charging_flag_error ||
		l_di->scp_stop_charging_flag_info ||
		(l_di->sysfs_enable_charger == 0))
		direct_charge_set_stage_status(DC_STAGE_DEFAULT);
	else if (l_di->pri_inversion)
		hwlog_info("pri_inversion,set state in function exit\n");
	else
		direct_charge_set_stage_status(DC_STAGE_CHARGE_DONE);

#ifdef CONFIG_WIRELESS_CHARGER
	if (wltx_need_disable_wired_dc())
		direct_charge_set_disable_flags(DC_SET_DISABLE_FLAGS,
			DC_DISABLE_WIRELESS_TX);
#endif /* CONFIG_WIRELESS_CHARGER */

	/*
	 * fix a sc adapter hardware issue:
	 * adapter has not output voltage when direct charger charge done
	 * we will set the adapter voltage to 5500mv
	 * when adapter voltage up to 7500mv on charge done stage
	 */
	if (l_di->reset_adap_volt_enabled &&
		(l_di->dc_stage == DC_STAGE_CHARGE_DONE) &&
		(l_di->adaptor_vset > 7500)) {
		l_di->adaptor_vset = 5500;
		direct_charge_set_adapter_voltage(l_di->adaptor_vset);
		usleep_range(200000, 201000); /* delay 200ms at least */
		direct_charge_get_adapter_voltage(&vadp);
		hwlog_info("set 5500mv when charge done, vadp=%d\n", vadp);
	}

	ret = l_di->ls_ops->ls_enable(0);
	if (ret)
		hwlog_err("ls enable fail\n");

	ret = adapter_set_default_state(ADAPTER_PROTOCOL_SCP);
	if (ret)
		hwlog_err("set adapter default state fail\n");

	pd_dpm_notify_direct_charge_status(false);

	l_di->can_stop_kick_wdt = 1;

	if (l_di->scp_work_on_charger) {
		direct_charge_adapter_protocol_power_supply(DISABLE);
		charge_set_hiz_enable(HIZ_MODE_DISABLE);
	}

	/* 1: enable discharge with device */
	if (l_di->ls_ops->ls_discharge) {
		ret = l_di->ls_ops->ls_discharge(1);
		if (ret)
			hwlog_err("ls enable discharge fail\n");
		else
			hwlog_info("ls enable discharge succ\n");
	}

	if (pmic_vbus_irq_is_enabled())
		direct_charger_disconnect_update_charger_type();

	/* switch to normal charging path */
	direct_charge_switch_charging_path(PATH_NORMAL);

	/* 0: disable discharge with device */
	if (l_di->ls_ops->ls_discharge) {
		ret = l_di->ls_ops->ls_discharge(0);
		if (ret)
			hwlog_err("ls disable discharge fail\n");
		else
			hwlog_info("ls disable discharge succ\n");
	}

	msleep(20); /* delay 20ms */

	/* judging whether the adapter is disconnect */
	direct_charge_get_vbus_vbat(&vbus, &vbat);
	if ((vbus < VBUS_ON_THRESHOLD) ||
		(vbat - vbus) > VBAT_VBUS_DIFFERENCE) {
		hwlog_info("direct charger disconnect\n");

		if (!power_cmdline_is_factory_mode())
			direct_charge_set_disable_flags(DC_CLEAR_DISABLE_FLAGS,
				DC_DISABLE_SYS_NODE);

		direct_charge_set_disable_flags(DC_CLEAR_DISABLE_FLAGS,
			DC_DISABLE_WIRELESS_TX);
		direct_charge_reset_para_in_stop(lvc_di);
		direct_charge_reset_para_in_stop(sc_di);

		direct_charge_disconnect_send_uevent();
#ifdef CONFIG_WIRELESS_CHARGER
		direct_charger_disconnect_event();
#endif /* CONFIG_WIRELESS_CHARGER */

		/*
		 * if pd not available, when pmic regn connected
		 * do disconnect here
		 */
		if (pmic_vbus_is_connected())
			pmic_vbus_disconnect_process();
	}

	ret = l_di->ls_ops->ls_exit();
	if (ret)
		hwlog_err("ls exit fail\n");

	ret = l_di->bi_ops->exit();
	if (ret)
		hwlog_err("bi exit fail\n");

	ret = adapter_soft_reset_master(ADAPTER_PROTOCOL_SCP);
	if (ret)
		hwlog_err("soft reset master fail\n");

	/* cancel threshold timer and work */
	hrtimer_cancel(&l_di->calc_thld_timer);
	cancel_work_sync(&l_di->calc_thld_work);

	/* cancel watchdog timer and work */
	hrtimer_cancel(&l_di->kick_wtd_timer);
	cancel_work_sync(&l_di->kick_wtd_work);

	/* reset parameter and release resource */
	direct_charge_set_stop_charging_flag(0);
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
	l_di->max_adaptor_cur = 0;
	direct_charge_wake_unlock();
	l_di->scp_stop_charging_complete_flag = 1;
	l_di->adaptor_test_result_type = TYPE_OTHER;
	l_di->working_mode = UNDEFINED_MODE;

	blocking_notifier_call_chain(&dc_notify_head,
		DC_STATUS_STOP_CHARGE, NULL);
	if (l_di->pri_inversion) {
		direct_charge_set_stage_status(DC_STAGE_DEFAULT);
		charge_request_charge_monitor();
	}
}
