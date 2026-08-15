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

#include <huawei_platform/power/direct_charger.h>
#include <huawei_platform/power/wired_channel_switch.h>
#include <linux/random.h>
#include <huawei_platform/power/power_mesg.h>
#include <huawei_platform/usb/hw_usb.h>
#include <huawei_platform/power/battery_voltage.h>
#include <huawei_platform/power/huawei_battery_temp.h>
#include <huawei_platform/usb/hw_pd_dev.h>

#define DC_AF_INFO_NL_OPS_NUM			1
#define DC_AF_WAIT_CT_TIMEOUT		1000
#define DC_AF_KEY_LEN     (SCP_ADAPTOR_DIGEST_LEN*2+1)
#define DC_SUPER_ICO_CURRENT	4000
#define DC_SUPER_ICO_BATT_TEMP  25

#define HWLOG_TAG direct_charge
HWLOG_REGIST();

static BLOCKING_NOTIFIER_HEAD(dc_notify_head);
static struct direct_charge_device *g_di;
static int g_power_ct_service_ready;
static u8 dc_af_key[DC_AF_KEY_LEN];
struct direct_charge_pps_ops *g_pps_ops;
struct direct_charge_cd_ops *g_cd_ops;
static int direct_charge_mode;

static bool not_support_direct;
static int g_dc_need_wired_sw_off = 1;

static bool g_is_abnormal_scp_adp;

/*PT set adator test result*/
extern void chg_set_adaptor_test_result(enum adaptor_name charger_type, enum test_state result);
extern int coul_get_battery_voltage_uv(void);
static long c_offset_a;
static long c_offset_b;
int direct_charge_resistance_handler(int res);

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
		ret = -EPERM;
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
		ret = -EPERM;
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

bool direct_charge_get_abnormal_adp_flag(void)
{
	return g_is_abnormal_scp_adp;
}

void direct_charge_set_abnormal_adp_flag(bool flag)
{
	g_is_abnormal_scp_adp = flag;
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

	return blocking_notifier_chain_register(&dc_notify_head, nb);
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

	if (!wake_lock_active(&l_di->charging_lock)) {
		wake_lock(&l_di->charging_lock);
		hwlog_info("wake lock\n");
	}
}

static void direct_charge_wake_unlock(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	if (wake_lock_active(&l_di->charging_lock)) {
		wake_unlock(&l_di->charging_lock);
		hwlog_info("wake unlock\n");
	}
}

/* get the stage of direct charge */
enum scp_stage_type direct_charge_get_stage_status(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return SCP_STAGE_DEFAULT;

	return l_di->scp_stage;
}

/* set the stage of direct charge */
void direct_charge_set_stage_status(enum scp_stage_type stage_type)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	l_di->scp_stage = stage_type;

	hwlog_info("set_stage_status: stage=%d\n", l_di->scp_stage);
}

void direct_charge_set_stage_status_default(void)
{
	direct_charge_set_stage_status(SCP_STAGE_DEFAULT);
}

void direct_charge_set_local_mode(int set, int dc_mode)
{
	if (set == AND_SET)
		direct_charge_mode &= dc_mode;

	if (set == OR_SET)
		direct_charge_mode |= dc_mode;
}

int direct_charge_get_local_mode(void)
{
	return direct_charge_mode;
}

int direct_charge_get_adapter_type(void)
{
	int support_mode = 0;
	int adapter_type = 0;

	adapter_get_support_mode(&support_mode);
	hwlog_info("support_mode=%d\n", support_mode);

	/* 1st check 10v2a & 20V3P25A */
	if (support_mode & SC_MODE) {
		adapter_get_adp_type(&adapter_type);
		if (adapter_type == ADP_B_TYPE1)
			return ADAPTER_TYPE_10V2A;

		if (adapter_type == ADP_B_TYPE1_65W)
			return ADAPTER_TYPE_20V3P25A;
	}

	/* 2nd check 10v4a */
	if ((support_mode & SC_MODE) && (support_mode & LVC_MODE))
		return ADAPTER_TYPE_10V4A;

	/* 3st check 5v5a */
	if (support_mode & LVC_MODE)
		return ADAPTER_TYPE_5V5A;

	return ADAPTER_TYPE_UNKNOWN;
}

int direct_charge_ops_is_valid(struct direct_charge_device *di)
{
	if (!di) {
		hwlog_err("di is null\n");
		return INVALID;
	}

	if (!di->pps_ops ||
		!di->pps_ops->power_supply_enable) {
		hwlog_err("pps_ops is null\n");
		return INVALID;
	}

	if (!di->ls_ops ||
		!di->ls_ops->ls_init ||
		!di->ls_ops->ls_exit ||
		!di->ls_ops->is_ls_close ||
		!di->ls_ops->get_ls_id ||
		!di->ls_ops->ls_enable ||
		!di->ls_ops->watchdog_config_ms) {
		hwlog_err("ls_ops is null\n");
		return INVALID;
	}

	if (!di->bi_ops ||
		!di->bi_ops->get_bat_current ||
		!di->bi_ops->exit ||
		!di->bi_ops->get_bat_btb_voltage ||
		!di->bi_ops->get_vbus_voltage) {
		hwlog_err("bi_ops is null\n");
		return INVALID;
	}

	return VALID;
}

int direct_charge_is_failed(void)
{
	struct direct_charge_device *lvc_di = NULL;
	struct direct_charge_device *sc_di = NULL;
	int lvc_ret;
	int sc_ret;

	lvc_ret = lvc_get_di(&lvc_di);
	sc_ret = sc_get_di(&sc_di);

	if (!lvc_ret) {
		if (lvc_di->error_cnt >= DC_ERR_CNT_MAX || not_support_direct) {
			hwlog_info("lvc error exceed retry max\n");
			return 1;
		} else {
			return 0;
		}
	}

	if (!sc_ret) {
		if (sc_di->error_cnt >= DC_ERR_CNT_MAX || not_support_direct) {
			hwlog_info("sc error exceed retry max\n");
			return 1;
		} else {
			return 0;
		}
	}

	return 0;
}

void direct_charge_send_normal_charging_uevent(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	/* clear quick_charging and super_charging flag */
	l_di->quick_charge_flag = 0;
	l_di->super_charge_flag = 0;

	direct_charger_connect_send_uevent();
}

void direct_charge_send_quick_charging_uevent(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	/* clear super_charging flag and set quick_charging flag */
	l_di->quick_charge_flag = 1;
	l_di->super_charge_flag = 0;

	direct_charger_connect_send_uevent();
}

void direct_charge_send_super_charging_uevent(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	/* clear quick_charging flag and set super_charging flag */
	l_di->quick_charge_flag = 0;
	l_di->super_charge_flag = 1;

	direct_charger_connect_send_uevent();
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

	hwlog_err("current temp(temp=%d) is illegal\n", value);
	return 0;
}

/* get the maximum current allowed by direct charging at specified resist */
static int direct_charge_resist_handler(int value)
{
	int i;
	struct direct_charge_device *l_di = direct_charge_get_di();

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
			if (value >= l_di->resist_para[i].resist_min &&
				value < l_di->resist_para[i].resist_max)
				return l_di->resist_para[i].resist_cur_max;
		}
	} else if (l_di->cc_cable_detect_ok == 1) {
		for (i = 0; i < DC_RESIST_LEVEL; ++i) {
			if (value >= l_di->std_resist_para[i].resist_min &&
				value < l_di->std_resist_para[i].resist_max)
				return l_di->std_resist_para[i].resist_cur_max;
		}
	}

	hwlog_err("current resist(cable=%d, resist=%d) is illegal\n",
		l_di->cc_cable_detect_ok, value);
	return 0;
}

int direct_charge_pre_check_battery_temp(void)
{
	int bat_temp = 0;
	int bat_temp_cur_max;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return 0;

	huawei_battery_temp(BAT_TEMP_MIXED, &bat_temp);
	bat_temp_cur_max = direct_charge_battery_temp_handler(bat_temp);

	if (bat_temp_cur_max == 0) {
		hwlog_info("can not do direct charging, temp=%d\n", bat_temp);
		return 0;
	}

	return 1;
}

int direct_charge_pre_check_battery_voltage(void)
{
	int volt_max = hw_battery_voltage(BAT_ID_MAX);
	int volt_min = hw_battery_voltage(BAT_ID_MIN);
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return 0;

	if (volt_min < l_di->min_dc_bat_vol ||
		volt_max > l_di->max_dc_bat_vol) {
		hwlog_info("can not do direct charging, v_min=%d v_max=%d\n",
			volt_min, volt_max);
		return 0;
	}

	return 1;
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
		hwlog_err("stop_charging_flag_error is set\n");
		return -1;
	}

	return 0;
}

static int direct_charge_get_bat_voltage(int *value)
{
	int btb_vol;
	int package_vol;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di || !l_di->bi_ops)
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

static int direct_charge_get_adapter_voltage(int *value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	if (direct_charge_get_stop_charging_flag())
		return -1;

	if (adapter_get_output_voltage(value)) {
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

	if (l_di->adaptor_vendor_id == IWATT_ADAPTER)
		return direct_charge_get_device_ibus(value);

	if (adapter_get_output_current(value)) {
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

	if (adapter_get_output_current_set(value)) {
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

	if (adapter_get_max_current(value)) {
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

	if (adapter_get_inside_temp(value)) {
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

static int direct_charge_set_adapter_voltage(int value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();
	int l_value = value;

	if (!l_di)
		return -1;

	if (direct_charge_get_stop_charging_flag())
		return -1;

	hwlog_info("set adapter_volt=%d,max_volt=%d\n",
		l_value, l_di->max_adaptor_vset);

	if (l_value > l_di->max_adaptor_vset)
		l_value = l_di->max_adaptor_vset;

	if (adapter_set_output_voltage(l_value)) {
		direct_charge_fill_eh_buf(l_di->dsm_buff,
			sizeof(l_di->dsm_buff),
			DC_EH_SET_APT_VOLT, NULL);
		direct_charge_set_stop_charging_flag(1);

		hwlog_err("set adapter voltage fail\n");
		return -1;
	}

	return 0;
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
		l_value, l_di->max_adaptor_iset);

	if (l_value > l_di->max_adaptor_iset)
		l_value = l_di->max_adaptor_iset;

	if (adapter_set_output_current(l_value)) {
		direct_charge_fill_eh_buf(l_di->dsm_buff,
			sizeof(l_di->dsm_buff),
			DC_EH_SET_APT_CURR, NULL);
		direct_charge_set_stop_charging_flag(1);

		hwlog_err("set adapter current fail\n");
		return -1;
	}

	return 0;
}

int direct_charge_init_adapter_and_device(void)
{
	struct adapter_init_data sid;
	char tmp_buf[ERR_NO_STRING_SIZE] = { 0 };
	int ret;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	sid.scp_mode_enable = 1;
	sid.vset_boundary = l_di->max_adaptor_vset;
	sid.iset_boundary = l_di->max_adaptor_iset;
	sid.init_adaptor_voltage = l_di->init_adapter_vset;
	sid.watchdog_timer = 3; /* 3seconds */

	pd_dpm_notify_direct_charge_status(true);

	ret = adapter_set_init_data(&sid);
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
	int retry = 3; /* retry 3times */
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

static int direct_charge_check_usb_port_leakage_current(void)
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
	case IWATT_ADAPTER:
		l_di->adaptor_iset = 400; /* default 400ma */
		if (direct_charge_set_adapter_current(l_di->adaptor_iset))
			return -1;

		/* delay 100ms */
		msleep(100);

		adapter_get_port_leakage_current_flag(&leak_current);
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
	int adjust_times = MAX_TIMES_FOR_SET_ADAPTER_VOL_30;
	char tmp_buf[ERR_NO_STRING_SIZE] = { 0 };
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	soc = hisi_battery_capacity();
	direct_charge_get_bat_sys_voltage(&vbat);
	max_dc_bat_vol = l_di->max_dc_bat_vol;

	if (l_di->open_path_vol_select)
		max_dc_bat_vol = vbat_max > max_dc_bat_vol ?
			vbat_max : max_dc_bat_vol;

	/* first: set voltage and current to adapter */
	l_di->adaptor_vset = vbat * l_di->dc_volt_ratio + l_di->init_delt_vset;
	if (l_di->max_adaptor_vset < l_di->adaptor_vset)
		l_di->adaptor_vset = l_di->max_adaptor_vset;
	l_di->adaptor_iset = CURRENT_SET_FOR_RES_DETECT_1000_MA;
	direct_charge_set_adapter_voltage(l_di->adaptor_vset);
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

	hwlog_info("[%d]: Vapt_set=%d, Ibus_ls=%d\n",
		adjust_times, l_di->adaptor_vset, ibus);

	/* third: adjust adapter voltage until current to 800ma */
	while (ibus < MIN_CURRENT_FOR_RES_DETECT_800_MA) {
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
		hwlog_info("current battery capacity is %d, over threshold(%d)\n",
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
	int retry = 3; /* retry 3times */
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

	/* keep communication with loadswitch within 1 second */
	if (direct_charge_get_device_close_status())
		return -1;

	/* delay 500ms */
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
	rpath_th = l_di->full_path_res_threshold;

	hwlog_info("average Rpath=%d, Rpath_threshold=[%d,%d]\n",
		rpath, -rpath_th, rpath_th);

	if (rpath <= rpath_th) {
		resist_cur_max = direct_charge_resist_handler(rpath);
		hwlog_err("resist_cur_max=%d\n", resist_cur_max);
		if ((l_di->cc_cable_detect_ok == 0) && (resist_cur_max != 0)) {
			if (pd_dpm_get_ctc_cable_flag())
				l_di->max_current_for_ctc_cable =
					resist_cur_max;
			else
				l_di->max_current_for_none_standard_cable =
					resist_cur_max;
		}
		if (l_di->cc_cable_detect_ok == 0)
			direct_charge_send_ico_uevent();

		hwlog_err("full path resistance check succ\n");
		return 0;
	}

	/* process error report */
	if (l_di->cc_cable_detect_ok == 1) {
		snprintf(tmp_buf, sizeof(tmp_buf),
			"full_res(%d) is out of[%d, %d]\n",
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

	l_di->direct_charge_succ_flag = DC_ERROR_ADAPTOR_VOLTAGE_ACCURACY;
	ret = direct_charge_check_adapter_voltage_accuracy();
	if (ret)
		return -1;

	l_di->direct_charge_succ_flag = DC_ERROR_USB_PORT_LEAKAGE_CURRENT;
	ret = direct_charge_check_usb_port_leakage_current();
	if (ret)
		return -1;

	l_di->direct_charge_succ_flag = DC_ERROR_OPEN_CHARGE_PATH;
	ret = direct_charge_open_charging_path();
	if (ret)
		return -1;

	l_di->direct_charge_succ_flag = DC_ERROR_FULL_REISISTANCE;
	ret = direct_charge_check_full_path_resistance();
	if (ret)
		return -1;

	return 0;
}

static inline void direct_charge_entry_func(struct direct_charge_device *di, int mode, int type)
{
	di->adaptor_test_result_type = type;
	di->scp_mode = mode;
	direct_charge_set_di(di);
	chg_set_adaptor_test_result(type,DETECT_SUCC);
	if (mode == SC_MODE) {
		sc_mode_check();
		if (is_in_scp_charging_stage() == IN_SCP_CHARGING_STAGE)
			blocking_notifier_call_chain(&dc_notify_head,
				SC_STATUS_CHARGING, NULL);
	} else if (mode == LVC_MODE) {
		lvc_mode_check();
		if (is_in_scp_charging_stage() == IN_SCP_CHARGING_STAGE)
			blocking_notifier_call_chain(&dc_notify_head,
				LVC_STATUS_CHARGING, NULL);
	}
	return;
}

bool direct_charge_check_sc_mode(void)
{
	struct direct_charge_device *di = g_di;
	if (di == NULL)
	{
		return false;
	}
	return (direct_charge_get_stage_status() >= SCP_STAGE_CHARGE_INIT);
}

void direct_charge_check(void)
{
	int lvc_ret = 0;
	int sc_ret = 0;
	int adap_mode = UNDEFINED_MODE;
	int local_mode= 0;
	int adp_type;
	bool cc_vbus_short = false;
	bool cc_moisture_status = false;
	bool cc_safe = false;
	struct direct_charge_device *lvc_di = NULL;
	struct direct_charge_device *sc_di = NULL;
	enum charge_done_type charge_done_status = get_charge_done_type();
	struct direct_charge_device *di = g_di;

	if (!di) {
		hwlog_err("di is NULL\n");
		return;
	}

	if (CHARGE_DONE == charge_done_status) {
		hwlog_info("charge done!");
		return;
	}

	lvc_ret = lvc_get_di(&lvc_di);
	sc_ret = sc_get_di(&sc_di);

	if (lvc_ret && sc_ret) {
		hwlog_info("local does not support direct_charge!");
		return;
	}

	if (sc_di) {
		if (SCP_STAGE_CHARGE_DONE == sc_di->scp_stage) {
			hwlog_info("sc charge done!");
			return;
		}
	}

	if(lvc_di) {
		if (SCP_STAGE_CHARGE_DONE == lvc_di->scp_stage) {
			hwlog_info("lvc charge done!");
			return;
		}
	}

	local_mode = direct_charge_get_local_mode();
	scp_adaptor_type_detect(&adap_mode);

	if (di->adaptor_detect_by_voltage == 0) {
		adp_type = direct_charge_get_adapter_type();
		if (adp_type == ADAPTER_TYPE_20V3P25A) {
			adap_mode = SC_MODE;
			hwlog_info("65W adp, set adap_mode %d\n", SC_MODE);
		}
	}

	if(UNDEFINED_MODE == adap_mode)
	{
		hwlog_info("undefined adaptor mode!");
		return;
	}
	hwlog_info("direct_charge [adaptor mode] = [%d], [local mode] = [%d]!\n", adap_mode, local_mode);

	/* cc rp 3.0 can not do high voltage charge */
	cc_vbus_short = pd_dpm_check_cc_vbus_short();

	if (cc_vbus_short)
		hwlog_err("cc match rp3.0, can not do sc charge\n");

	cc_moisture_status = pd_dpm_get_cc_moisture_status();
	if (cc_moisture_status)
		hwlog_err("cc moisture detected\n");

	cc_safe = !cc_vbus_short && !cc_moisture_status;

	if (((local_mode & adap_mode) & SC_MODE) && sc_di &&
		!sc_di->dc_err_report_flag && cc_safe) {
		not_support_direct = false;
		direct_charge_entry_func(sc_di, SC_MODE, TYPE_SC);
	} else if (((local_mode & adap_mode) & LVC_MODE) && lvc_di &&
		!lvc_di->dc_err_report_flag &&
		(!lvc_di->cc_protect || cc_safe)) {
		not_support_direct = false;
		direct_charge_entry_func(lvc_di, LVC_MODE, TYPE_SCP);
	} else {
		not_support_direct = true;
		hwlog_info("neither sc nor lvc matched!");
	}

	return;
}

void direct_charge_set_adapter_default_param(void)
{
	adapter_set_default_param();
}

int direct_charge_disable_usbpd(bool disable)
{
	return adapter_set_usbpd_enable(!disable);
}

void direct_charge_force_disable_dc_path(void)
{
	struct direct_charge_device *di = g_di;

	if (!di || !di->ls_ops || !di->ls_ops->ls_enable)
		return;

	di->ls_ops->ls_enable(0);
}

int direct_charge_pre_check(void)
{
	int adap_mode = UNDEFINED_MODE;
	int local_mode= 0;

	local_mode = direct_charge_get_local_mode();
	scp_adaptor_type_detect(&adap_mode);

	if (adap_mode & local_mode) {
		hwlog_info("%s support\n", __func__);
		return 0;
	} else {
		hwlog_info("%s not support\n", __func__);
		return -1;
	}
}

static int select_super_ico_bat_limit(void)
{
	int i;
	int bat_limit;
	int tbat = DC_SUPER_ICO_BATT_TEMP;
	char *bat_brand = NULL;
	struct direct_charge_device *di = NULL;

	di = g_di;

	if (!di) {
		hwlog_err("%s: di is NULL\n", __func__);
		return -1;
	}

	bat_limit = di->orig_volt_para_p[0].volt_info[0].cur_th_high;
	bat_brand = hisi_battery_brand();

	hwlog_info("%s: bat_brand = %s, di->stage_group_size = %d\n",
		__func__, bat_brand, di->stage_group_size);

	for (i = 0; i < di->stage_group_size; i++) {
		if (!di->orig_volt_para_p[i].bat_info.parse_ok)
			continue;
		if (!strstr(bat_brand, di->orig_volt_para_p[i].bat_info.batid))
			continue;
		if (tbat >= di->orig_volt_para_p[i].bat_info.temp_high ||
			tbat < di->orig_volt_para_p[i].bat_info.temp_low)
			continue;
		bat_limit = di->orig_volt_para_p[i].volt_info[0].cur_th_high;
		return bat_limit;
	}

	hwlog_info("%s: bat_limit=%d\n", __func__, bat_limit);
	return bat_limit;
}

void direct_charge_send_ico_uevent(void)
{
	struct direct_charge_device *di = NULL;
	int max_current;
	int cable_limit;
	int bat_limit;
	int adapter_type;

	if (!g_di) {
		hwlog_err("%s: g_di is NULL\n", __func__);
		return;
	}
	di = g_di;

	if (pd_dpm_get_ctc_cable_flag())
		cable_limit = di->max_current_for_ctc_cable;
	else
		cable_limit = di->max_current_for_none_standard_cable;

	bat_limit = select_super_ico_bat_limit();
	if (di->cc_cable_detect_enable && di->cc_cable_detect_ok == 0)
		max_current = bat_limit < cable_limit ? bat_limit : cable_limit;
	else
		max_current = bat_limit;

	adapter_type = direct_charge_get_adapter_type();
	if (adapter_type == ADAPTER_TYPE_10V2A) {
		direct_charge_send_quick_charging_uevent();
		return;
	}

	if (max_current >= di->super_ico_current)
		direct_charge_send_super_charging_uevent();
	else
		direct_charge_send_quick_charging_uevent();
}

int direct_charge_get_cutoff_normal_flag(void)
{
	struct direct_charge_device *di = NULL;
	if (NULL == g_di)
	{
		hwlog_err("%s g_di is NULL\n", __func__);
		return 0;
	}
	di = g_di;

	return di->cutoff_normal_flag;
}

int is_in_scp_charging_stage(void)
{
	struct direct_charge_device *di = NULL;
	if (NULL == g_di)
	{
		hwlog_err("%s g_di is NULL\n", __func__);
		return NOT_IN_SCP_CHARGING_STAGE;
	}
	di = g_di;

	if (SCP_STAGE_CHARGING == di->scp_stage)
	{
		hwlog_info("in direct charge progress!\n");
		return IN_SCP_CHARGING_STAGE;
	}
	return NOT_IN_SCP_CHARGING_STAGE;
}

int is_scp_stop_charging_complete(void)
{
	struct direct_charge_device *di = NULL;
	if (NULL == g_di)
	{
		hwlog_err("%s g_di is NULL\n", __func__);
		return 1;
	}
	di = g_di;
	return (1 == di->scp_stop_charging_complete_flag);
}

int scp_retry_pre_operate(enum scp_retry_operate_type type, struct direct_charge_device *di)
{
	int ret	= -1;

	switch (type) {
	case SCP_RETRY_OPERATE_RESET_ADAPTER:
			hwlog_info("send scp adapter reset cmd \n");
			ret = adapter_soft_reset_slave();
		break;
	case SCP_RETRY_OPERATE_RESET_CHIP:
			hwlog_info("scp_chip_reset \n");
			ret = adapter_soft_reset_master();
			msleep(2000);
		break;
	default:
		break;
	}
	return ret;
}

void scp_power_control(int enable)
{
	int ret = 0;
	struct direct_charge_device *di = NULL;
	if (NULL == g_di)
	{
		hwlog_err("%s g_di is NULL\n", __func__);
		return ;
	}
	di = g_di;
	/* power control for SCP communication */
	ret = di->pps_ops->power_supply_enable(enable);
	if(ret)
	{
		hwlog_err("[%s]:fail, status = %d!\n", __func__,enable);
		return;
	}
	hwlog_err("[%s]: success, status = %d!\n", __func__,enable);
	return;
}
static int turn_off_wired_channel(void)
{
	if (g_dc_need_wired_sw_off)
		return wired_chsw_set_wired_channel(WIRED_CHANNEL_CUTOFF);
	else
		return 0;
}
static int turn_on_wired_channel(void)
{
	if (g_dc_need_wired_sw_off)
		return wired_chsw_set_wired_channel(WIRED_CHANNEL_RESTORE);
	else
		return 0;
}
int restore_normal_charge(void)
{
	int ret = SUCC;
	struct direct_charge_device *di = NULL;
	if (NULL == g_di)
	{
		hwlog_err("%s g_di is NULL\n", __func__);
		return FAIL;
	}
	di = g_di;
	hwlog_err("%s \n", __func__);
	msleep(WAIT_LS_DISCHARGE); /*need to wait loadswitch discharge*/
	/*no need to check the return val, here when ovp_en set fail ,we do note return*/
	hw_usb_ldo_supply_disable(HW_USB_LDO_CTRL_DIRECT_CHARGE);

	ret = turn_on_wired_channel();
	if (ret)
		ret = FAIL;
	else
		ret = SUCC;

	if (pmic_vbus_irq_is_enabled()) {
		restore_pluggin_pluggout_interrupt();
	}
	return ret;
}
int cutoff_normal_charge(void)
{
	int ret = SUCC;
	struct direct_charge_device *di = NULL;
	if (NULL == g_di)
	{
		hwlog_err("%s g_di is NULL\n", __func__);
		return -1;
	}
	di = g_di;
	if (pmic_vbus_irq_is_enabled()) {
		di->cutoff_normal_flag = 1;
		ignore_pluggin_and_pluggout_interrupt();
	}
	hw_usb_ldo_supply_enable(HW_USB_LDO_CTRL_DIRECT_CHARGE);
	if (di->scp_work_on_charger) {
		scp_power_control(ENABLE);
		charge_set_hiz_enable(HIZ_MODE_ENABLE);
	}
	msleep(100);
	ret = turn_off_wired_channel();
	hwlog_err("%s \n", __func__);

	return ret;
}
int scp_adaptor_set_output_enable(int enable)
{
	int ret, i;
	struct direct_charge_device *di = NULL;
	if (NULL == g_di)
	{
		hwlog_err("%s g_di is NULL\n", __func__);
		return -1;
	}
	di = g_di;

    	for (i = 0; i < 3; i++) {
		ret = adapter_set_output_enable(enable);
		if (!ret)
			break;
	}
	if (ret) {
		hwlog_err("[%s]: direct charge set output voltage fail !!\n", __func__);
	}
	return ret;
}
int _scp_adaptor_detect(struct direct_charge_device *di)
{
	int ret;
	int i;
	int support_mode = 0;

	ret = adapter_detect_adapter_support_mode(&support_mode);
	if (SCP_ADAPTOR_DETECT_FAIL == ret)
	{
		for (i = 0; i < 3 && SCP_ADAPTOR_DETECT_FAIL == ret; ++i)
		{
			/*check if the adapter has been pulled out */
			if(direct_charge_get_stage_status() ==
				SCP_STAGE_DEFAULT) {
				hwlog_info("[%s]the adapter has been pulled out,stop adapter detect!\n", __func__);
				return -1;
			}
			if ((scp_retry_pre_operate(SCP_RETRY_OPERATE_RESET_ADAPTER, di)) < 0)
			{
				hwlog_err("reset adapter failed	\n");
				break;
			}
			ret = adapter_detect_adapter_support_mode(&support_mode);
		}
		if (SCP_ADAPTOR_DETECT_FAIL == ret)
		{
			/*check if the adapter has been pulled out */
			if (direct_charge_get_stage_status() ==
				SCP_STAGE_DEFAULT) {
				hwlog_info("[%s]the adapter has been pulled out,stop adapter detect!\n", __func__);
				return -1;
			}

			/* reset scp chip and try again	*/
			if ((scp_retry_pre_operate(SCP_RETRY_OPERATE_RESET_CHIP, di)) == 0)
			{
				ret = adapter_detect_adapter_support_mode(&support_mode);
			}
			else
			{
				hwlog_err("%s : scp_retry_pre_operate failed \n", __func__);
			}
		}
	}
	hwlog_info("%s : scp adaptor detect support_mode=%x ret = %d \n", __func__, support_mode, ret);
	return ret;
}
void direct_charge_double_56k_cable_detect(void)
{
	int ret;
	bool cc_moisture_status = false;
	int adapter_type;
	enum cur_cap c_cap;
	struct direct_charge_device *di = NULL;

	if (!g_di) {
		hwlog_err("%s g_di is NULL\n", __func__);
		return ;
	}
	di = g_di;
	if (!di || !di->cd_ops || !di->cd_ops->cable_detect) {
		hwlog_err("[%s]NULL direct charge cable detect ops!\n", __func__);
		direct_charge_set_stage_status(SCP_STAGE_DEFAULT);
		return;
	}
	if (di->cc_cable_detect_ok) {
		return;
	}
	/* 10v2a no need check cable */
	adapter_type = direct_charge_get_adapter_type();
	if (adapter_type == ADAPTER_TYPE_10V2A) {
		di->cc_cable_detect_ok = 1; /* detect success flag */
		di->full_path_res_threshold = di->standard_cable_full_path_res_max;
		direct_charge_send_ico_uevent();
		return;
	}

	ret = di->cd_ops->cable_detect();
	cc_moisture_status = pd_dpm_get_cc_moisture_status();
	c_cap = pd_dpm_get_cvdo_cur_cap();

	if (ret && !cc_moisture_status && (c_cap < PD_DPM_CURR_5A)) {
		di->cc_cable_detect_ok = 0;
		if (pd_dpm_get_ctc_cable_flag())
			di->full_path_res_threshold =
				di->ctc_cable_full_path_res_max;
		else
			di->full_path_res_threshold =
				di->full_path_res_max;
		hwlog_info("%s:cable detect fail!\n",__func__);
		if (di->is_show_ico_first)
			direct_charge_send_ico_uevent();
	} else {
		di->cc_cable_detect_ok = 1;
		di->full_path_res_threshold = di->standard_cable_full_path_res_max;
		direct_charge_send_ico_uevent();
		hwlog_info("%s:cable detect ok!\n",__func__);
	}

}

int scp_adaptor_detect(void)
{
	int ret = 0;
	struct direct_charge_device *di = NULL;
	if (NULL == g_di)
	{
		hwlog_err("%s g_di is NULL\n", __func__);
		return -1;
	}
	di = g_di;
	ret = _scp_adaptor_detect(di);
	/*try again in the next loop*/
	if (SCP_ADAPTOR_DETECT_SUCC != ret) {
		direct_charge_set_stage_status(SCP_STAGE_DEFAULT);
		return -1;
	}
	chg_set_adaptor_test_result(di->adaptor_test_result_type,DETECT_SUCC);
	return 0;
}

static inline bool scp_adapter_vol_check(struct direct_charge_device *di, int max, int min)
{
	int batt_num = hw_battery_get_series_num();

	if (NULL == di) {
		hwlog_err("%s g_di is NULL\n", __func__);
		return false;
	}

	hwlog_info("max:%d,min:%d,batt_num:%d,max_dc_bat_vol:%d,min_dc_bat_vol:%d\n",
		max, min, batt_num, di->max_dc_bat_vol,di->min_dc_bat_vol);
	hwlog_info("dc_volt_ratio:%d,init_delt_vset:%d\n",
		di->dc_volt_ratio, di->init_delt_vset);

	if (max > (di->max_dc_bat_vol * batt_num * di->dc_volt_ratio + di->init_delt_vset) &&
		min < (di->min_dc_bat_vol * batt_num * di->dc_volt_ratio + di->init_delt_vset)) {
		return true;
	} else {
		return false;
	}
}

static int scp_adaptor_redetect_by_voltage(int mode)
{
	int ret = 0;
	int re_mode = 0;
	int vout_max = 0;
	int vout_min = 0;
	int local_mode = direct_charge_get_local_mode();
	struct direct_charge_device *sc_di = NULL;
	struct direct_charge_device *lvc_di = NULL;

	/* if mode is not B class adapter, return raw mode */
	if (!(mode & (SC_MODE | LVC_MODE))) {
		return mode;
	}
	if (!(local_mode & (SC_MODE | LVC_MODE))) {
		return mode;
	}

	ret = adapter_get_max_voltage(&vout_max);
	if (ret || vout_max <= 0) {
		hwlog_err("get adapter vout fail: max = %d, ret = %d\n",
			vout_max, ret);
		return mode;
	}

	ret = adapter_get_min_voltage(&vout_min);
	if (ret || vout_min <= 0) {
		hwlog_err("get adapter vout fail: min = %d, ret = %d\n",
			vout_min, ret);
		return mode;
	}

	sc_get_di(&sc_di);
	if ((local_mode & SC_MODE) && sc_di) {
		if (scp_adapter_vol_check(sc_di, vout_max, vout_min)) {
			re_mode |= SC_MODE;
		}
	}

	lvc_get_di(&lvc_di);
	if ((local_mode & LVC_MODE) && lvc_di) {
		if (scp_adapter_vol_check(lvc_di, vout_max, vout_min)) {
			re_mode |= LVC_MODE;
		}
	}

	hwlog_info("%s:vout_max:%d,vout_min:%d,re_mode:0x%x!\n",
		__func__, vout_max, vout_min,re_mode);

	if (!re_mode)
		direct_charge_set_abnormal_adp_flag(true);

	return re_mode;
}

int scp_adaptor_type_detect(int *mode)
{
	int adaptor_support = 0;
	int ret = 0;
	struct direct_charge_device *di = NULL;

	if (!g_di) {
		hwlog_err("%s g_di is NULL\n", __func__);
		return -1;
	}
	di = g_di;

	di->direct_charge_succ_flag = DC_ERROR_ADAPTOR_DETECT;
	ret = _scp_adaptor_detect(di);

	if (ret) {
		hwlog_info("%s:scp_adaptor_detect fail!\n", __func__);
		direct_charge_set_stage_status(SCP_STAGE_DEFAULT);
		return -1;
	}

	adapter_get_support_mode(&adaptor_support);
	if (di->adaptor_detect_by_voltage)
		*mode = scp_adaptor_redetect_by_voltage(adaptor_support);
	else
		*mode = adaptor_support;

	return 0;
}


void scp_cable_detect(void)
{
	int ret = 0;
	struct direct_charge_device *di = NULL;

	if (NULL == g_di)
	{
		hwlog_err("%s g_di is NULL\n", __func__);
		return ;
	}
	di = g_di;
	direct_charge_double_56k_cable_detect();

	ret = adapter_get_device_info();
	if (ret)
	{
		hwlog_err("get adaptor info failed\n");
		return;
	}

	if (adapter_get_chip_vendor_id(&di->adaptor_vendor_id) < 0)
	{
		hwlog_err("get adaptor vendor id failed\n");
		direct_charge_set_stage_status(SCP_STAGE_DEFAULT);
		return;
	}

	adapter_show_device_info();

	hwlog_info("adaptor_vendor_id= 0x%x\n", di->adaptor_vendor_id);
	direct_charge_set_stage_status(SCP_STAGE_SWITCH_DETECT);
}

#ifdef CONFIG_HUAWEI_POWER_MESG_INTERFACE
static int direct_charge_af_srv_on_cb(void)
{
	g_power_ct_service_ready = 1;
	return 0;
}

static int direct_charge_af_cb(unsigned char version, void * data, int len)
{
	struct direct_charge_device *di = g_di;

	if (NULL == di)
	{
		hwlog_err("%s g_di is NULL\n", __func__);
		return -1;
	}

	if (len != 1)
		return -1;

	di->dc_antifake_result = *(int *) data;
	complete(&di->dc_af_completion);
	hwlog_info("direct_charge_af_cb called! dc_antifake_result = %d\n", di->dc_antifake_result);
	return 0;
}

static const easy_cbs_t dc_af_ops[DC_AF_INFO_NL_OPS_NUM] = {
	{
		.cmd = POWER_CMD_ADAPTOR_ANTIFAKE_HASH,
		.doit = direct_charge_af_cb,
	}
};

static power_mesg_node_t dc_af_info_node = {
	.target = POWERCT_PORT,
	.name = "DC_AF",
	.ops = dc_af_ops,
	.n_ops = DC_AF_INFO_NL_OPS_NUM,
	.srv_on_cb = direct_charge_af_srv_on_cb,
};
#endif

static int direct_charge_af_calc_hash(void)
{
	struct direct_charge_device *di = g_di;

	if (NULL == di)
	{
		hwlog_err("%s g_di is NULL\n", __func__);
		return -1;
	}

	/*if power ct service not ready , we asume the powerct serivce is dead, return hash calc ok anyway*/
	if (0 == g_power_ct_service_ready) {
		hwlog_err("%s g_power_ct_service not ready\n", __func__);
		return 0;
	}

	hwlog_info("%s. start\n", __func__);

#ifdef CONFIG_HUAWEI_POWER_MESG_INTERFACE
	if(power_easy_send(&dc_af_info_node, POWER_CMD_ADAPTOR_ANTIFAKE_HASH, 0,
                           dc_af_key, DC_AF_KEY_LEN)) {
		hwlog_err("mesg send failed in %s.\n", __func__);
	}
#endif

	if (!wait_for_completion_timeout(&di->dc_af_completion, DC_AF_WAIT_CT_TIMEOUT)) {
		/*if time out happend, we asume the powerct serivce is dead, return hash calc ok anyway*/
		hwlog_err("wait_for_completion_timeout timeout!!! %s.\n", __func__);
		return 0;
	}
	hwlog_info("%s. end\n", __func__);

	/*if not timeout , base on the hash calc result return the antifake result*/
	if (di->dc_antifake_result == 0) {
		hwlog_err("%s. dc_antifake_result = %d\n", __func__, di->dc_antifake_result);
		return -1;
	} else {
		hwlog_info("%s. dc_antifake_result = %d\n", __func__, di->dc_antifake_result);
		return 0;
	}
}

int direct_charge_gen_nl_init(struct platform_device *pdev)
{
	int ret = 0;
	struct direct_charge_device *di = g_di;

	if (NULL == di)
	{
		hwlog_err("%s g_di is NULL\n", __func__);
		return -1;
	}

	if (!di->adaptor_antifake_check_enable)
		return 0;

#ifdef CONFIG_HUAWEI_POWER_MESG_INTERFACE
	ret = power_easy_node_register(&dc_af_info_node);
	if(ret) {
		hwlog_err("power_genl_add_op failed!\n");
	}
#endif

	return ret;
}

int do_adpator_antifake_check(void)
{
	int ret = 0;
	int adapter_type;

	struct direct_charge_device *di = NULL;

	if (NULL == g_di)
	{
		hwlog_err("%s g_di is NULL\n", __func__);
		return -1;
	}
	di = g_di;

	if (!di->adaptor_antifake_check_enable)
		return 0;

	/* the type adp no need antifake check */
	adapter_type = direct_charge_get_adapter_type();
	if (adapter_type == ADAPTER_TYPE_10V2A)
		return 0;

	memset(dc_af_key, 0x00, DC_AF_KEY_LEN);

	ret = adapter_auth_encrypt_start(di->adaptor_antifake_key_index, dc_af_key, DC_AF_KEY_LEN);
	if (ret)
	{
		goto err;
	}

	ret = direct_charge_af_calc_hash();
	if (ret)
	{
		goto err;
	}

err:
	ret |= adapter_auth_encrypt_release();
	if (ret)
	{
		return -1;
	}
	return 0;
}

int is_support_scp(void)
{
	/*return 0 means support scp*/
	if (adapter_get_protocol_register_state())
	{
		hwlog_err("not support scp!\n");
		return 1;
	}
	direct_charge_set_stage_status(SCP_STAGE_ADAPTER_DETECT);
	return 0;
}

int set_direct_charger_disable_flags(int val, int type)
{
	int i;
	int disable = 0;
	struct direct_charge_device *lvc_di = NULL;
	struct direct_charge_device *sc_di = NULL;

	lvc_get_di(&lvc_di);
	sc_get_di(&sc_di);

	if(!lvc_di && !sc_di) {
		hwlog_err("NULL direct_charge_device pointer found in %s.\n", __func__);
		return -1;
	}

	if(type < 0 || type >= __MAX_DISABLE_DIRECT_CHAGER){
		hwlog_err("set direct charger to %d with wrong type(%d) in %s.\n",
					val, type, __func__);
		return -1;
	}

	if (lvc_di) {
		lvc_di->sysfs_disable_charger[type] = val;
		for( i = 0; i < __MAX_DISABLE_DIRECT_CHAGER; i++){
			disable |= lvc_di->sysfs_disable_charger[i];
		}
		lvc_di->sysfs_enable_charger = !disable;
	}

	if (sc_di) {
		sc_di->sysfs_disable_charger[type] = val;
		for( i = 0; i < __MAX_DISABLE_DIRECT_CHAGER; i++){
			disable |= sc_di->sysfs_disable_charger[i];
		}
		sc_di->sysfs_enable_charger = !disable;
	}

	return 0;
}

static int get_vbus_vbat(int *vbus, int *vbat)
{
	struct direct_charge_device *di = NULL;
	if (NULL == g_di)
	{
		hwlog_err("%s g_di is NULL\n", __func__);
		return -1;
	}
	di = g_di;

	if (di->ls_ops->ls_status)
	{
		if (0 != di->ls_ops->ls_status())
		{
			msleep(DIRECT_CHARGER_LS_RECOVERY_DELAY);
			*vbus = get_charger_vbus_vol();
			*vbat = coul_get_battery_voltage_uv();
			*vbat = *vbat / DIRECT_CHARGER_COUL_CURRENT_UNIT_DEVIDE;
			hwlog_err("%s: ls(sc) fatal error!!!  vbus = %d  vbat = %d!\n", __func__, *vbus, *vbat);
			return 0;
		}
	}

	direct_charge_get_device_vbus(vbus);
	direct_charge_get_bat_voltage(vbat);
	hwlog_info("%s:ls vbus = %d  vbat = %d!\n", __func__, *vbus, *vbat);

	return 0;
}

void scp_stop_charging_para_reset(struct direct_charge_device* di)
{
	if (di) {
		di->error_cnt = 0;
		di->otp_cnt = 0;
		di->adaptor_otp_cnt = 0;
		di->reverse_ocp_cnt = 0;
		di->dc_open_retry_cnt = 0;
		di->full_path_resistance = ERROR_RESISTANCE;
		di->scp_adaptor_detect_flag = SCP_ADAPTOR_NOT_DETECT;
		di->direct_charge_succ_flag = DC_ERROR_ADAPTOR_DETECT;
		di->quick_charge_flag = 0;
		di->super_charge_flag = 0;
		di->cc_cable_detect_ok = 0;
		di->dc_err_report_flag = FALSE;
		di->sc_conv_ocp_count = 0;
		direct_charge_clean_eh_buf(di->dsm_buff, sizeof(di->dsm_buff));
	}
}

void scp_stop_charging(void)
{
	int ret;
	int vbus_vol = 0;
	int vbat = 0;
	int voltage_max_threshold = 7500; /*max voltage check threshold  mv*/
	struct direct_charge_device *lvc_di = NULL;
	struct direct_charge_device *sc_di = NULL;
	struct direct_charge_device *di = NULL;
	int adp_vol = 0;

	if (NULL == g_di)
	{
		hwlog_err("%s g_di is NULL\n", __func__);
		return ;
	}
	di = g_di;
	if (direct_charge_get_stage_status() != SCP_STAGE_CHARGE_DONE) {
		di->scp_adaptor_detect_flag = SCP_ADAPTOR_NOT_DETECT;
	}
	if (di->stop_charging_flag_error)
	{
		di->error_cnt += 1;
	}
	if (di->stop_charging_flag_error ||
		di->scp_stop_charging_flag_info ||
		(di->sysfs_enable_charger == 0)) {
		direct_charge_set_stage_status(SCP_STAGE_DEFAULT);
	}
	else
	{
		direct_charge_set_stage_status(SCP_STAGE_CHARGE_DONE);
	}
#ifdef CONFIG_WIRELESS_CHARGER
	if (wltx_need_disable_wired_dc())
		set_direct_charger_disable_flags(
			DIRECT_CHARGER_SET_DISABLE_FLAGS,
			DIRECT_CHARGER_WIRELESS_TX);
#endif

	/*
	 * fix a sc adaptor issue:
	 * adaptor has not output voltage when direct charger charge done
	 */
	if (di->reset_adap_volt_enabled &&
		(di->scp_stage == SCP_STAGE_CHARGE_DONE) &&
		(di->adaptor_vset > voltage_max_threshold)) {
		di->adaptor_vset = 5500; /* set adaptor voltage to 5.5v */
		direct_charge_set_adapter_voltage(di->adaptor_vset);
		usleep_range(200000, 201000); /* delay 200ms at least */
		direct_charge_get_adapter_voltage(&adp_vol);
		hwlog_info("set volt to default when charge done, volt=%d\n",
			adp_vol);
	}

	ret = di->ls_ops->ls_enable(0);
	if (ret)
	{
		hwlog_err("[%s]: ls enable fail!\n", __func__);
	}

	ret = adapter_set_default_state();
	if (ret)
	{
		hwlog_err("[%s]: scp exit fail!\n", __func__);
	}
	pd_dpm_notify_direct_charge_status(false);

	di->can_stop_kick_wdt = 1;

	if (di->scp_work_on_charger)
	{
		scp_power_control(DISABLE);
		charge_set_hiz_enable(HIZ_MODE_DISABLE);
	}
	if (di->ls_ops->ls_discharge)
	{
		ret = di->ls_ops->ls_discharge(1);
		if (ret)
		{
			hwlog_err("[%s]: ls discharge fail!\n", __func__);
		}
		else
		{
			hwlog_info("[%s]: ls discharge succ!\n", __func__);
		}
	}
	if (pmic_vbus_irq_is_enabled()) {
		direct_charger_disconnect_update_charger_type();
	}
	if (FAIL == restore_normal_charge())
	{
		hwlog_info("%s: restore normal charge fail!\n", __func__);
	}
	ret = di->ls_ops->ls_discharge(0);
	msleep(20);

	get_vbus_vbat(&vbus_vol, &vbat);

	if ((vbus_vol < VBUS_ON_THRESHOLD) || (vbat - vbus_vol) > VBAT_VBUS_DIFFERENCE)
	{
		hwlog_info("%s: vbat - vbus_vol = %d!\n", __func__, vbat - vbus_vol);
		hwlog_info("%s:direct charger disconnect!\n", __func__);

		lvc_get_di(&lvc_di);
		sc_get_di(&sc_di);

		if(!lvc_di && !sc_di) {
			hwlog_err("NULL direct_charge_device pointer found in %s.\n", __func__);
			return;
		}

		if (!strstr(saved_command_line, "androidboot.swtype=factory"))
			set_direct_charger_disable_flags(DIRECT_CHARGER_CLEAR_DISABLE_FLAGS,
                                             DIRECT_CHARGER_SYS_NODE);

		set_direct_charger_disable_flags
			(DIRECT_CHARGER_CLEAR_DISABLE_FLAGS, DIRECT_CHARGER_WIRELESS_TX);
		scp_stop_charging_para_reset(lvc_di);
		scp_stop_charging_para_reset(sc_di);
		direct_charger_disconnect_send_uevent();
#ifdef CONFIG_WIRELESS_CHARGER
		direct_charger_disconnect_event();
#endif
	}
	else
	{
		hwlog_info("%s:switch to normal charge!\n", __func__);
	}
	ret = di->ls_ops->ls_exit();
	if (ret)
	{
		hwlog_err("[%s]: ls exit fail!\n", __func__);
	}
	ret = di->bi_ops->exit();
	if (ret)
	{
		hwlog_err("[%s]: bi exit fail!\n", __func__);
	}
	ret = adapter_soft_reset_master();
	if (ret)
	{
		hwlog_err("[%s]: scp_chip_reset fail!\n", __func__);
	}
	hrtimer_cancel(&di->calc_thld_timer);
	cancel_work_sync(&di->calc_thld_work);

	hrtimer_cancel(&di->kick_wtd_timer);
	cancel_work_sync(&di->kick_wtd_work);

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
	di->ibat_abnormal_cnt = 0;
	di->max_adaptor_cur = 0;
	direct_charge_wake_unlock();
	di->scp_stop_charging_complete_flag = 1;
	di->adaptor_test_result_type = TYPE_OTHER;
	di->scp_mode = UNDEFINED_MODE;

	blocking_notifier_call_chain(&dc_notify_head,
		DC_STATUS_STOP_CHARGE, NULL);
}

int jump_stage_if_need(int cur_stage)
{
	int i;
	struct direct_charge_device *di = NULL;
	if (NULL == g_di)
	{
		hwlog_err("%s g_di is NULL\n", __func__);
		return -1;
	}
	di = g_di;

	for (i = 0; i < 2*di->stage_size; ++i)
	{
		if (-1 == di->stage_need_to_jump[i])
		{
			return cur_stage;
		}
		else
		{
			if (cur_stage == di->stage_need_to_jump[i])
			{
				hwlog_info("jump stage %d\n", cur_stage);
				return jump_stage_if_need(cur_stage + 1);
			}
		}
	}
	return cur_stage;
}

/*lint -save -e* */
void select_direct_charge_stage(void)
{
	int i;
	int vbat_th;
	int cur_stage = 0;
	int stage_size = 0;
	int vbat = 0;
	int ibat = 0;
	int iadaptor = 0;
	char tmp_buf[ERR_NO_STRING_SIZE] = { 0 };
	struct direct_charge_device *di = NULL;
	if (NULL == g_di)
	{
		hwlog_err("%s g_di is NULL\n", __func__);
		return ;
	}
	di = g_di;
	stage_size = di->stage_size;

	direct_charge_get_bat_voltage(&vbat);
	direct_charge_get_bat_current(&ibat);
	di->vbat = vbat;
	di->ibat = ibat;
	direct_charge_get_adapter_current(&iadaptor);
	if (iadaptor < di->ibat_abnormal_th)
	{
		di->ibat_abnormal_cnt++;
	}
	else
	{
		di->ibat_abnormal_cnt = 0;
	}
	if (di->ibat_abnormal_cnt > 10)
	{
		direct_charge_set_stop_charging_flag(1);
		snprintf(tmp_buf, sizeof(tmp_buf),"%s: ibat abnormal, stop direct charge\n", __func__);
		direct_charge_fill_eh_buf(di->dsm_buff,
			sizeof(di->dsm_buff),
			DC_EH_IBAT_ABNORMAL, tmp_buf);
		return;
	}
	di->pre_stage = di->cur_stage;
	for (i = stage_size - 1; i >=0; --i)
	{
		vbat_th = di->volt_para[i].vol_th + di->compensate_v;
		vbat_th = vbat_th > di->volt_para[stage_size - 1].vol_th ? di->volt_para[stage_size - 1].vol_th : vbat_th;
		if (vbat >= vbat_th && ibat <= di->volt_para[i].cur_th_low)
		{
			cur_stage = 2*i +2;
			break;
		}
		else if (vbat >= vbat_th)
		{
			cur_stage = 2*i +1;
			break;
		}
	}
	if (i < 0)
	{
		cur_stage = 0;
	}
	if (cur_stage < di->pre_stage)
	{
		cur_stage = di->pre_stage;
	}
	if (di->first_cc_stage_timer_in_min)
	{
		if (0 == cur_stage || 1 == cur_stage)
		{
			if (time_after(jiffies, di->first_cc_stage_timeout))
			{
				hwlog_info("first_cc_stage_timeout in %d min, set stage = 2\n",di->first_cc_stage_timer_in_min);
				cur_stage = 2;
			}
		}
	}
	if (cur_stage != di->cur_stage)
	{
		di->cur_stage = jump_stage_if_need(cur_stage);
	}
}

void select_direct_charge_stage_param(void)
{
	int i;
	int tbatt = 0;
	char* batt_brand = NULL;
	struct direct_charge_device *di = NULL;

	di = g_di;

	if (NULL == di)
	{
		hwlog_err("%s di is NULL\n", __func__);
		return;
	}

	batt_brand = hisi_battery_brand();
	huawei_battery_temp(BAT_TEMP_MIXED, &tbatt);

	hwlog_info("batt_brand = %s, tbatt = %d, di->stage_group_size = %d\n", batt_brand, tbatt, di->stage_group_size);

	for (i = 0; i < di->stage_group_size; i++)
	{
		if (di->orig_volt_para_p[i].bat_info.parse_ok) {
			if (strstr(batt_brand, di->orig_volt_para_p[i].bat_info.batid)) {
				if (tbatt < di->orig_volt_para_p[i].bat_info.temp_high && tbatt >= di->orig_volt_para_p[i].bat_info.temp_low) {
					di->stage_group_cur = i;
					di->stage_size = di->orig_volt_para_p[i].stage_size;
					memcpy(di->volt_para, di->orig_volt_para_p[i].volt_info, sizeof(di->volt_para));
					memcpy(di->orig_volt_para, di->orig_volt_para_p[i].volt_info, sizeof(di->orig_volt_para));
					hwlog_info("stage_group_cur= %d, batid = %s, temp high = %d, temp low = %d\n",  \
							i, di->orig_volt_para_p[i].bat_info.batid, di->orig_volt_para_p[i].bat_info.temp_high, di->orig_volt_para_p[i].bat_info.temp_low);
					return;
				}
			}
		}
	}

	di->stage_group_cur = 0;
	di->stage_size = di->orig_volt_para_p[0].stage_size;
	memcpy(di->volt_para, di->orig_volt_para_p[0].volt_info, sizeof(di->volt_para));
	memcpy(di->orig_volt_para, di->orig_volt_para_p[0].volt_info, sizeof(di->orig_volt_para));
}
void select_direct_charge_param(void)
{
	int cur_th_high;
	int max_adaptor_cur = 0;
	int vbat_th;
	int bat_temp_cur_max;
	int full_path_resist_cur_max;
	int full_path_resistance;
	int bat_temp = 0;
	int type = 0;
	int adapter_type;

	struct direct_charge_device *di = NULL;
	if (NULL == g_di)
	{
		hwlog_err("%s g_di is NULL\n", __func__);
		return;
	}
	di = g_di;

	huawei_battery_temp(BAT_TEMP_MIXED, &bat_temp);

	direct_charge_double_56k_cable_detect();
	bat_temp_cur_max = direct_charge_battery_temp_handler(bat_temp);
	if (0 == bat_temp_cur_max)
	{
		hwlog_info("%s : temp = %d, can not do direct charging \n", __func__, bat_temp);
		di->scp_stop_charging_flag_info = 1;
	}

	full_path_resistance = di->full_path_resistance >= 0 ? di->full_path_resistance : -di->full_path_resistance;
	full_path_resist_cur_max = direct_charge_resist_handler(
		full_path_resistance);

	if (direct_charge_get_adapter_max_current(&max_adaptor_cur))
		return;

	/* the type adp support gain curr */
	adapter_type = direct_charge_get_adapter_type();
	if (adapter_type == ADAPTER_TYPE_10V2A)
		max_adaptor_cur += di->gain_curr;

	if (di->use_5a) {
		if (max_adaptor_cur == 4500) /* 4500ma */
			max_adaptor_cur = di->volt_para[0].cur_th_high;
	}

	if (di->use_8a) {
		if (max_adaptor_cur == 4000) /* 4000ma */
			max_adaptor_cur = di->volt_para[0].cur_th_high;
	}

	if (max_adaptor_cur != di->max_adaptor_cur)
	{
		hwlog_info("%s : max_adaptor_cur = %d\n", __func__, max_adaptor_cur);
		di->max_adaptor_cur = max_adaptor_cur;
	}
	di->compensate_v = di->ibat*di->compensate_r/1000;
	vbat_th = di->volt_para[di->cur_stage/2].vol_th + di->compensate_v;
	di->cur_vbat_th = vbat_th < di->volt_para[di->stage_size - 1].vol_th ? vbat_th: di->volt_para[di->stage_size - 1].vol_th;

	/* cur_stage include cc and cv stage so divide 2 */
	cur_th_high = di->volt_para[di->cur_stage / 2].cur_th_high;

	if (di->cc_cable_detect_enable) {
		if (0 == di->cc_cable_detect_ok) {
			if (pd_dpm_get_ctc_cable_flag())
				cur_th_high = (cur_th_high >
					di->max_current_for_ctc_cable) ?
					di->max_current_for_ctc_cable :
					cur_th_high;
			else
				cur_th_high = (cur_th_high >
					di->max_current_for_none_standard_cable) ?
					di->max_current_for_none_standard_cable :
					cur_th_high;
		}

		hwlog_info("[%s]:cc_check_result = %d ,after_cc_check_cur_th_high = %d\n",
			__func__, di->cc_cable_detect_ok, cur_th_high);
	}
	cur_th_high = cur_th_high >  bat_temp_cur_max ? bat_temp_cur_max : cur_th_high;

	if (full_path_resist_cur_max != 0)
		cur_th_high = cur_th_high > full_path_resist_cur_max ?
			      full_path_resist_cur_max : cur_th_high;

	cur_th_high = cur_th_high >  di->dc_volt_ratio * max_adaptor_cur ? di->dc_volt_ratio * max_adaptor_cur : cur_th_high;
	di->cur_ibat_th_high = cur_th_high > di->sysfs_iin_thermal ? di->sysfs_iin_thermal: cur_th_high;
	di->cur_ibat_th_low = di->volt_para[di->cur_stage/2].cur_th_low;
}

void battery_aging_safe_policy(struct direct_charge_device *di,
				bool force_update)
{
	int ret, i, cur_level;
	AGING_SAFE_POLICY_TYPE basp = {0};

	if (NULL == di)
	{
		hwlog_err("%s direct_charge_device is NULL\n", __func__);
		return ;
	}

	ret = hisi_battery_aging_safe_policy(&basp);
	if (ret) {
		hwlog_err(BASP_TAG"[%s] get basp policy fail, ret:%d!\n", __func__, ret);
		return;
	}

	if ((di->stage_size < 1) || (di->stage_size > DC_VOLT_LEVEL)) {
		hwlog_err("stage_size %d out of range\n", di->stage_size);
		return;
	}

	cur_level = basp.level;

	if (cur_level != di->last_basp_level || force_update) {
		di->volt_para[di->stage_size -1].vol_th =
			di->orig_volt_para[di->stage_size -1].vol_th - basp.dc_volt_dec;
		for (i = 0; i < di->stage_size -1; i++) {
			di->volt_para[i].vol_th = di->orig_volt_para[i].vol_th < di->volt_para[di->stage_size -1].vol_th\
								? di->orig_volt_para[i].vol_th : di->volt_para[di->stage_size -1].vol_th;
		}
		switch (basp.cur_ratio_policy) {
		case BASP_RATIO_POLICY_ALL:
			di->volt_para[di->stage_size -1].cur_th_high =
						di->orig_volt_para[di->stage_size -1].cur_th_high * basp.cur_ratio/BASP_PARA_SCALE;
			di->volt_para[di->stage_size -1].cur_th_low =
						di->orig_volt_para[di->stage_size -1].cur_th_low;
			for (i = 0; i < di->stage_size -1; i++) {
				di->volt_para[i].cur_th_high = di->orig_volt_para[i].cur_th_high * basp.cur_ratio /BASP_PARA_SCALE;
				di->volt_para[i].cur_th_low = di->orig_volt_para[i].cur_th_low * basp.cur_ratio /BASP_PARA_SCALE;
			}
			break;
		case BASP_RATIO_POLICY_MAX:
			di->volt_para[0].cur_th_high = di->orig_volt_para[0].cur_th_high * basp.cur_ratio /BASP_PARA_SCALE;
			di->volt_para[0].cur_th_low = di->orig_volt_para[0].cur_th_low;
			for (i = 1; i < di->stage_size;i++) {
				di->volt_para[i].cur_th_high = di->orig_volt_para[i].cur_th_high <= di->volt_para[0].cur_th_high ?
							di->orig_volt_para[i].cur_th_high : di->volt_para[0].cur_th_high;
				di->volt_para[i].cur_th_low = di->orig_volt_para[i].cur_th_low;
			}
			break;
		default:
			break;
		}
		di->last_basp_level = cur_level;
		hwlog_info(BASP_TAG"cur_level = %d\n", cur_level);
		for (i = 0; i < di->stage_size; i++) {
			hwlog_info(BASP_TAG"volt_para[%d], vol_th:%d, cur_th_high:%d, cur_th_low:%d\n",
				i, di->volt_para[i].vol_th, di->volt_para[i].cur_th_high, di->volt_para[i].cur_th_low);
		}
	}
	return;
}
void direct_charge_regulation(void)
{
	char buf[ERR_NO_STRING_SIZE] = { 0 };
	int ret;
	struct direct_charge_device *di = NULL;
	int iadapt = 0;
	int iadapt_set = 0;
	int vbat;
	int ibat;
	int vbat_sh;
	int ibat_sh_high;
	int ibat_sh_low;

	if (NULL == g_di)
	{
		hwlog_err("%s g_di is NULL\n", __func__);
		return ;
	}
	di = g_di;
	direct_charge_get_adapter_current(&iadapt);
	direct_charge_get_adapter_current_set(&iadapt_set);
	direct_charge_get_adapter_voltage(&di->vadapt);
	direct_charge_get_adapter_temp(&di->tadapt);
	direct_charge_get_device_ibus(&di->ls_ibus);
	direct_charge_get_device_vbus(&di->ls_vbus);
	direct_charge_get_device_temp(&di->tls);
	di->iadapt = iadapt;
	vbat = di->vbat;
	ibat = di->ibat;
	vbat_sh = di->cur_vbat_th;
	ibat_sh_high = di->cur_ibat_th_high;
	ibat_sh_low = di->cur_ibat_th_low;

	hwlog_info("cur_stage = %d vbat = %d ibat = %d vbat_sh = %d ibat_sh_high = %d ibat_sh_low = %d vadp = %d iadap = %d ls_vbus = %d ls_ibus = %d iadapt_set = %d tadapt = %d tls = %d!\n",
			di->cur_stage, vbat, ibat, vbat_sh, ibat_sh_high, ibat_sh_low, di->vadapt, iadapt, di->ls_vbus, di->ls_ibus, iadapt_set, di->tadapt, di->tls);
	if (di->tls > (int)(di->max_tls))
	{
		snprintf(buf, sizeof(buf), "%s: tls = %d > %d, stop direct_charge!\n", __func__, di->tls, di->max_tls);
		direct_charge_fill_eh_buf(di->dsm_buff,
			sizeof(di->dsm_buff),
			DC_EH_TLS_ABNORMAL, buf);
		direct_charge_set_stop_charging_flag(1);
		return;
	}
	if (di->tadapt > (int)(di->max_tadapt))
	{
		snprintf(buf, sizeof(buf), "%s: tadapt(%d) > [%d], stop direct_charge!\n", __func__, di->tadapt, di->max_tadapt);
		di->adaptor_otp_cnt++;
		direct_charge_fill_eh_buf(di->dsm_buff,
			sizeof(di->dsm_buff),
			DC_EH_TADP_ABNORMAL, buf);
		if (di->adaptor_otp_cnt >= ADAPTOR_OTP_CNT) {
			di->adaptor_otp_cnt = ADAPTOR_OTP_CNT;
			power_dsm_dmd_report(POWER_DSM_BATTERY, DSM_DIRECT_CHARGE_ADAPTER_OTP, buf);
		}
		direct_charge_set_stop_charging_flag(1);
		return;
	}

	/* keep communication with loadswitchwithin 1 second */
	ret = direct_charge_get_device_close_status();
	if (ret)
		return;

	/* 2: cc & cv statge */
	if (di->cur_stage % 2) {
		if (vbat > vbat_sh)
		{
			di->adaptor_vset += di->dc_volt_ratio * (vbat_sh - vbat);
			direct_charge_set_adapter_voltage(di->adaptor_vset);
			return;
		}
		if (iadapt > ibat_sh_high/di->dc_volt_ratio)
		{
			di->adaptor_vset -= di->vstep;
			direct_charge_set_adapter_voltage(di->adaptor_vset);
			return;
		}
		if (ibat > ibat_sh_high)
		{
			di->adaptor_vset -= di->vstep;
			direct_charge_set_adapter_voltage(di->adaptor_vset);
			return;
		}
		if (ibat > ibat_sh_high - di->delta_err)
		{
			hwlog_info("do nothing!\n");
			return;
		}
		if (di->adaptor_iset < (ibat_sh_high - 1000)/di->dc_volt_ratio)
		{
			di->adaptor_iset += 1000/di->dc_volt_ratio;
			direct_charge_set_adapter_current(di->adaptor_iset);
			return;
		}
		else if (di->adaptor_iset < ibat_sh_high/di->dc_volt_ratio)
		{
			di->adaptor_iset = ibat_sh_high;
			direct_charge_set_adapter_current(di->adaptor_iset);
			return;
		}
		else
		{
			if (iadapt < (ibat_sh_high - di->delta_err)/di->dc_volt_ratio)
			{
				di->adaptor_vset += di->vstep;
				direct_charge_set_adapter_voltage(
					di->adaptor_vset);
				return;
			}
		}
	}
	else
	{
		if (iadapt > ibat_sh_high/di->dc_volt_ratio)
		{
			di->adaptor_vset -= di->vstep;
			direct_charge_set_adapter_voltage(di->adaptor_vset);
			return;
		}
		if (ibat > ibat_sh_high)
		{
			di->adaptor_vset -= di->vstep;
			direct_charge_set_adapter_voltage(di->adaptor_vset);
			return;
		}
		if (ibat > ibat_sh_high - di->delta_err)
		{
			hwlog_info("do nothing!\n");
			return;
		}
		if (di->adaptor_iset < (ibat_sh_high - 1000)/di->dc_volt_ratio)
		{
			di->adaptor_iset += 1000/di->dc_volt_ratio;
			direct_charge_set_adapter_current(di->adaptor_iset);
			return;
		}
		else if (di->adaptor_iset < ibat_sh_high/di->dc_volt_ratio)
		{
			di->adaptor_iset = ibat_sh_high/di->dc_volt_ratio;
			direct_charge_set_adapter_current(di->adaptor_iset);
			return;
		}
		else
		{
			if (iadapt < (ibat_sh_high - di->delta_err)/di->dc_volt_ratio)
			{
				di->adaptor_vset += di->vstep;
				direct_charge_set_adapter_voltage(
					di->adaptor_vset);
				return;
			}
		}

	}
}

void direct_charge_control_work(struct work_struct *work)
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
		hwlog_info("direct charge stop\n");
		scp_stop_charging();
		return;
	}

	if (DOUBLE_SIZE * di->stage_size == di->cur_stage) {
		hwlog_info("cur_stage=%d, vbat=%d, ibat=%d\n",
			di->cur_stage, di->vbat, di->ibat);
		hwlog_info("direct charge done\n");
		direct_charge_set_stage_status(SCP_STAGE_CHARGE_DONE);
		scp_stop_charging();
		return;
	}

	direct_charge_regulation();

	interval = di->charge_control_interval;
	hrtimer_start(&di->control_timer,
		ktime_set(interval / MSEC_PER_SEC,
			(interval % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);
}

void direct_charge_calc_thld_work(struct work_struct *work)
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
		hwlog_info("direct charge stop, stop calc threshold\n");
		return;
	}

	battery_aging_safe_policy(di, false);
	select_direct_charge_stage();
	select_direct_charge_param();

	if (DOUBLE_SIZE * di->stage_size == di->cur_stage) {
		hwlog_info("direct charge done, stop calc threshold\n");
		return;
	}

	interval = di->threshold_caculation_interval;
	hrtimer_start(&di->calc_thld_timer,
		ktime_set(interval / MSEC_PER_SEC,
			(interval % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);
}

void direct_charge_kick_wtd_work(struct work_struct *work)
{
	struct direct_charge_device *di = NULL;
	int interval;
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

	/* keep communication with the adaptor within 1 second */
	direct_charge_get_bat_current(&ibat);

	interval = KICK_WATCHDOG_TIME;
	hrtimer_start(&di->kick_wtd_timer,
		ktime_set(interval / MSEC_PER_SEC,
			(interval % MSEC_PER_SEC) * USEC_PER_SEC),
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
	enum scp_stage_type stage = direct_charge_get_stage_status();

	if (!nb) {
		hwlog_err("nb is null\n");
		return NOTIFY_OK;
	}

	di = container_of(nb, struct direct_charge_device, fault_nb);
	if (!di) {
		hwlog_err("di is null\n");
		return NOTIFY_OK;
	}

	if (stage < SCP_STAGE_SECURITY_CHECK ||
		stage == SCP_STAGE_CHARGE_DONE) {
		hwlog_err("ignore notifier when not in direct charging\n");
		return NOTIFY_OK;
	}

	di->charge_fault = (enum charge_fault_type)event;
	di->fault_data = (struct nty_data *)data;
	schedule_work(&di->fault_work);

	return NOTIFY_OK;
}

void scp_start_charging(void)
{
	int interval;
	struct direct_charge_device *di = NULL;

	hwlog_info("%s \n",__func__);

	if (NULL == g_di)
	{
		hwlog_err("%s g_di is NULL\n", __func__);
		return ;
	}
	di = g_di;

	hisi_coul_charger_event_rcv(VCHRG_START_AC_CHARGING_EVENT);

	direct_charge_wake_lock();
	if (di->first_cc_stage_timer_in_min)
	{
		/*8A maximum 5 min, ap will not suspend in direct charge mode, so use jiffies */
		hwlog_info("%s start timing\n",__func__);
		di->first_cc_stage_timeout = jiffies + msecs_to_jiffies(di->first_cc_stage_timer_in_min*60*MSEC_PER_SEC);
	}

	select_direct_charge_stage_param();
	select_direct_charge_stage();
	battery_aging_safe_policy(di, true);

	if (di->cur_stage == 0) {
		di->volt_para[0].cur_th_high -= di->sc_conv_ocp_count * 500;
		hwlog_info("%s sc_conv_ocp_count = %d, cur_th_high[0] = %d \n",__func__, di->sc_conv_ocp_count, di->volt_para[0].cur_th_high);
	}
	chg_set_adaptor_test_result(di->adaptor_test_result_type,PROTOCOL_FINISH_SUCC);
	direct_charge_set_stage_status(SCP_STAGE_CHARGING);
	select_direct_charge_param();
	di->ls_ops->watchdog_config_ms(WATCHDOG_TIMEOUT);
	di->can_stop_kick_wdt = 0;

	interval = di->charge_control_interval;
	hrtimer_start(&di->control_timer,
		ktime_set(interval / MSEC_PER_SEC,
			(interval % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);

	interval = di->threshold_caculation_interval;
	hrtimer_start(&di->calc_thld_timer,
		ktime_set(interval / MSEC_PER_SEC,
			(interval % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);

	interval = KICK_WATCHDOG_TIME;
	hrtimer_start(&di->kick_wtd_timer,
		ktime_set(interval / MSEC_PER_SEC,
			(interval % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);
}

enum direct_charge_mode direct_charge_get_adaptor_mode(void)
{
	if (g_di) {
		return g_di->scp_mode;
	} else {
		return UNDEFINED_MODE;
	}
}

/*lint -save -e* */
void direct_charge_update_cutoff_flag(void)
{
	struct direct_charge_device *lvc_di = NULL;
	struct direct_charge_device *sc_di = NULL;
	int cutoff_normal_flag = 0;

	lvc_get_di(&lvc_di);
	sc_get_di(&sc_di);

	if (lvc_di)
		cutoff_normal_flag |= lvc_di->cutoff_normal_flag;
	if (sc_di)
		cutoff_normal_flag |= sc_di->cutoff_normal_flag;

	if (pmic_vbus_irq_is_enabled() && cutoff_normal_flag) {
		if (lvc_di) {
			if (lvc_di->cutoff_normal_flag) {
				lvc_di->cutoff_normal_flag = 0;
				hwlog_info("clr lvc cutoff_normal_flag\n");
			}
		}
		if (sc_di) {
			if (sc_di->cutoff_normal_flag) {
				sc_di->cutoff_normal_flag = 0;
				hwlog_info("clr sc cutoff_normal_flag\n");
			}
		}
	} else {
		if (lvc_di)
			scp_stop_charging_para_reset(lvc_di);
		if (sc_di)
			scp_stop_charging_para_reset(sc_di);
	}
}

static void direct_charge_reset_para(struct direct_charge_device *di)
{
	if (!di)
		return;

	di->full_path_resistance = ERROR_RESISTANCE;
	di->scp_adaptor_detect_flag = SCP_ADAPTOR_NOT_DETECT;
	di->direct_charge_succ_flag = DC_ERROR_ADAPTOR_DETECT;
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
	di->adaptor_otp_cnt = 0;
	di->reverse_ocp_cnt = 0;
	di->dc_err_report_flag = FALSE;
	direct_charge_clean_eh_buf(di->dsm_buff, sizeof(di->dsm_buff));
}

void direct_charge_stop_charging(void)
{
	struct direct_charge_device *lvc_di = NULL;
	struct direct_charge_device *sc_di = NULL;

	lvc_get_di(&lvc_di);
	sc_get_di(&sc_di);

	if (!lvc_di && !sc_di) {
		hwlog_err("lvc_di or sc_di is null\n");
		return;
	}

	if (!strstr(saved_command_line, "androidboot.swtype=factory")) {
		set_direct_charger_disable_flags(
			DIRECT_CHARGER_CLEAR_DISABLE_FLAGS,
			DIRECT_CHARGER_SYS_NODE);
	}

	direct_charge_reset_para(lvc_di);
	direct_charge_reset_para(sc_di);
}

static void direct_charge_parse_gain_current_para(struct device_node *np,
	struct direct_charge_device *di)
{
	int ret;

	ret = of_property_read_u32(np, "gain_curr", &(di->gain_curr));
	if (ret) {
		hwlog_err("gain_curr dts read failed\n");
		di->gain_curr = 0;
	}
	hwlog_info("gain_curr=%d\n", di->gain_curr);
}

static void direct_charge_parse_adapter_antifake_para(struct device_node *np,
	struct direct_charge_device *di)
{
	int ret;

	ret = of_property_read_u32(np, "adaptor_antifake_check_enable",
		&(di->adaptor_antifake_check_enable));
	if (ret) {
		hwlog_err("adaptor_antifake_check_enable dts read failed\n");
		di->adaptor_antifake_check_enable = 0;
		return;
	}
	hwlog_info("adaptor_antifake_check_enable=%d\n",
		di->adaptor_antifake_check_enable);

	if (di->adaptor_antifake_check_enable != 1) {
		di->adaptor_antifake_check_enable = 0;
		hwlog_err("adapter_antifake not support\n");
		return;
	}

	ret = of_property_read_u32(np, "adaptor_antifake_key_index",
		&(di->adaptor_antifake_key_index));
	if (ret) {
		hwlog_err("adaptor_antifake_key_index dts read failed\n");
		di->adaptor_antifake_key_index = 1;
	}

	/*
	 * we set public key as default key in factory mode,
	 * otherwise we get key index from dts
	 */
	if (strstr(saved_command_line, "androidboot.swtype=factory"))
		di->adaptor_antifake_key_index = 1;

	hwlog_info("adaptor_antifake_key_index=%d\n",
		di->adaptor_antifake_key_index);
}

static int direct_charge_parse_volt_para(struct device_node *np,
	struct direct_charge_device *di, int group)
{
	int i;
	int row;
	int col;
	int ret;
	int array_len;
	int idata;
	const char *tmp_string = NULL;
	int j = group;
	char *volt_para = NULL;

	volt_para = di->orig_volt_para_p[j].bat_info.volt_para_index;

	array_len = of_property_count_strings(np, volt_para);
	di->orig_volt_para_p[j].stage_size = array_len / DC_PARA_VOLT_TOTAL;
	hwlog_info("%s array_len=%d, stage_size=%d\n", volt_para,
		array_len, di->orig_volt_para_p[j].stage_size);

	if ((array_len <= 0) ||
		(array_len % DC_PARA_VOLT_TOTAL != 0) ||
		(array_len > DC_VOLT_LEVEL * DC_PARA_VOLT_TOTAL)) {
		hwlog_err("%s invalid\n", volt_para);
		return -EINVAL;
	}

	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np, volt_para,
			i, &tmp_string);
		if (ret) {
			hwlog_err("%s dts read failed\n", volt_para);
			return -EINVAL;
		}

		ret = kstrtoint(tmp_string, DECIMAL_BASE, &idata);
		if (ret) {
			hwlog_err("get kstrtoint failed\n");
			return -EINVAL;
		}

		row = i / DC_PARA_VOLT_TOTAL;
		col = i % DC_PARA_VOLT_TOTAL;

		switch (col) {
		case DC_PARA_VOL_TH:
#ifdef CONFIG_SCHARGER_V300
			if (is_hi6523_cv_limit())
				idata -= HI6523_CV_CUT;
#endif /* CONFIG_SCHARGER_V300 */

			di->orig_volt_para_p[j].volt_info[row].vol_th = idata;
			break;

		case DC_PARA_CUR_TH_HIGH:
			di->orig_volt_para_p[j].volt_info[row].cur_th_high = idata;
			break;

		case DC_PARA_CUR_TH_LOW:
			di->orig_volt_para_p[j].volt_info[row].cur_th_low = idata;
			break;
		}
	}

	di->orig_volt_para_p[j].bat_info.parse_ok = 1;

	for (i = 0; i < di->orig_volt_para_p[j].stage_size; i++) {
		hwlog_info("%s[%d]=%d %d %d\n", volt_para, i,
			di->orig_volt_para_p[j].volt_info[i].vol_th,
			di->orig_volt_para_p[j].volt_info[i].cur_th_high,
			di->orig_volt_para_p[j].volt_info[i].cur_th_low);
	}

	return 0;
}

static void direct_charge_parse_group_volt_para(struct device_node *np,
	struct direct_charge_device *di)
{
	int i;

	di->stage_size = 0;

	for (i = 0; i < di->stage_group_size; i++) {
		if (direct_charge_parse_volt_para(np, di, i))
			return;
	}

	di->stage_size = di->orig_volt_para_p[0].stage_size;
}

static void direct_charge_parse_bat_para(struct device_node *np,
	struct direct_charge_device *di)
{
	int i;
	int row;
	int col;
	int ret;
	int array_len;
	int idata;
	const char *tmp_string = NULL;

	di->stage_group_cur = 0;

	array_len = of_property_count_strings(np, "bat_para");
	hwlog_info("bat_para array_len=%d\n", array_len);

	if ((array_len <= 0) ||
		(array_len % DC_PARA_BAT_TOTAL != 0) ||
		(array_len > DC_VOLT_GROUP_MAX * DC_PARA_BAT_TOTAL)) {
		di->stage_group_size = 1;
		 /* default temp_high is 45 centigrade */
		di->orig_volt_para_p[0].bat_info.temp_high = 45;
		 /* default temp_low is 10 centigrade */
		di->orig_volt_para_p[0].bat_info.temp_low = 10;
		strncpy(di->orig_volt_para_p[0].bat_info.batid,
			"default", DC_BAT_BRAND_LEN_MAX - 1);
		strncpy(di->orig_volt_para_p[0].bat_info.volt_para_index,
			"volt_para", DC_VOLT_NODE_LEN_MAX - 1);

		hwlog_err("bat_para invalid\n");
		return;
	}

	di->stage_group_size = array_len / DC_PARA_BAT_TOTAL;

	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np, "bat_para",
			i, &tmp_string);
		if (ret) {
			hwlog_err("bat_para dts read failed\n");
			return;
		}

		row = i / DC_PARA_BAT_TOTAL;
		col = i % DC_PARA_BAT_TOTAL;

		switch (col) {
		case DC_PARA_BAT_ID:
			strncpy(di->orig_volt_para_p[row].bat_info.batid,
				tmp_string, DC_BAT_BRAND_LEN_MAX - 1);
			break;

		case DC_PARA_TEMP_HIGH:
			ret = kstrtoint(tmp_string, DECIMAL_BASE, &idata);
			if (ret) {
				hwlog_err("get kstrtoint failed\n");
				return;
			}

			/* must be (0, 50) centigrade */
			if (idata < 0 || idata > 50) {
				hwlog_err("temp_high(%d) invalid\n", idata);
				return;
			}

			di->orig_volt_para_p[row].bat_info.temp_high = idata;
			break;

		case DC_PARA_TEMP_LOW:
			ret = kstrtoint(tmp_string, DECIMAL_BASE, &idata);
			if (ret) {
				hwlog_err("get kstrtoint failed\n");
				return;
			}

			/* must be (0, 50) centigrade */
			if (idata < 0 || idata > 50) {
				hwlog_err("temp_low(%d) invalid\n", idata);
				return;
			}

			di->orig_volt_para_p[row].bat_info.temp_low = idata;
			break;

		case DC_PARA_INDEX:
			strncpy(di->orig_volt_para_p[row].bat_info.volt_para_index,
				tmp_string, DC_VOLT_NODE_LEN_MAX - 1);
			break;
		}
	}

	for (i = 0; i < di->stage_group_size; i++) {
		hwlog_info("bat_para[%d]=%s %d %d %s\n", i,
			di->orig_volt_para_p[i].bat_info.batid,
			di->orig_volt_para_p[i].bat_info.temp_low,
			di->orig_volt_para_p[i].bat_info.temp_high,
			di->orig_volt_para_p[i].bat_info.volt_para_index);
	}
}

static void direct_charge_parse_std_resist_para(struct device_node *np,
	struct direct_charge_device *di)
{
	int i;
	int row;
	int col;
	int ret;
	int array_len;
	int idata;
	const char *tmp_string = NULL;

	array_len = of_property_count_strings(np, "std_resist_para");
	hwlog_info("std_resist_para array_len=%d\n", array_len);

	if ((array_len <= 0) ||
		(array_len % DC_RESIST_TOTAL != 0) ||
		(array_len > DC_RESIST_LEVEL * DC_RESIST_TOTAL)) {
		hwlog_err("std_resist_para invalid\n");
		return;
	}

	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np, "std_resist_para",
			i, &tmp_string);
		if (ret) {
			hwlog_err("std_resist_para dts read failed\n");
			return;
		}

		ret = kstrtoint(tmp_string, DECIMAL_BASE, &idata);
		if (ret) {
			hwlog_err("get kstrtoint failed\n");
			return;
		}

		row = i / DC_RESIST_TOTAL;
		col = i % DC_RESIST_TOTAL;

		switch (col) {
		case DC_RESIST_MIN:
			di->std_resist_para[row].resist_min = idata;
			break;

		case DC_RESIST_MAX:
			di->std_resist_para[row].resist_max = idata;
			break;

		case DC_RESIST_CUR_MAX:
			di->std_resist_para[row].resist_cur_max = idata;
			break;
		}

		hwlog_info("std_resist_para[%d][%d]=%d\n", row, col, idata);
	}
}

static void direct_charge_parse_non_std_resist_para(struct device_node *np,
	struct direct_charge_device *di)
{
	int i;
	int row;
	int col;
	int ret;
	int array_len;
	int idata;
	const char *tmp_string = NULL;

	array_len = of_property_count_strings(np, "resist_para");
	hwlog_info("resist_para array_len=%d\n", array_len);

	if ((array_len <= 0) ||
		(array_len % DC_RESIST_TOTAL != 0) ||
		(array_len > DC_RESIST_LEVEL * DC_RESIST_TOTAL)) {
		hwlog_err("resist_para invalid\n");
		return;
	}

	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np, "resist_para",
			i, &tmp_string);
		if (ret) {
			hwlog_err("resist_para dts read failed\n");
			return;
		}

		ret = kstrtoint(tmp_string, DECIMAL_BASE, &idata);
		if (ret) {
			hwlog_err("get kstrtoint failed\n");
			return;
		}

		row = i / DC_RESIST_TOTAL;
		col = i % DC_RESIST_TOTAL;

		switch (col) {
		case DC_RESIST_MIN:
			di->resist_para[row].resist_min = idata;
			break;

		case DC_RESIST_MAX:
			di->resist_para[row].resist_max = idata;
			break;

		case DC_RESIST_CUR_MAX:
			di->resist_para[row].resist_cur_max = idata;
			break;
		}

		hwlog_info("resist_para[%d][%d]=%d\n", row, col, idata);
	}
}

static void direct_charge_parse_ctc_resist_para(struct device_node *np,
	struct direct_charge_device *di)
{
	int i;
	int row;
	int col;
	int ret;
	int array_len;
	int idata;
	const char *tmp_string = NULL;

	array_len = of_property_count_strings(np, "ctc_resist_para");
	hwlog_info("ctc_resist_para array_len=%d\n", array_len);

	if ((array_len <= 0) ||
		(array_len % DC_RESIST_TOTAL != 0) ||
		(array_len > DC_RESIST_LEVEL * DC_RESIST_TOTAL)) {
		hwlog_err("ctc_resist_para invalid\n");
		return;
	}

	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np, "ctc_resist_para",
			i, &tmp_string);
		if (ret) {
			hwlog_err("ctc_resist_para dts read failed\n");
			return;
		}

		/* 10: decimal base */
		ret = kstrtoint(tmp_string, 10, &idata);
		if (ret) {
			hwlog_err("get kstrtoint failed\n");
			return;
		}

		row = i / DC_RESIST_TOTAL;
		col = i % DC_RESIST_TOTAL;

		switch (col) {
		case DC_RESIST_MIN:
			di->ctc_resist_para[row].resist_min = idata;
			break;

		case DC_RESIST_MAX:
			di->ctc_resist_para[row].resist_max = idata;
			break;

		case DC_RESIST_CUR_MAX:
			di->ctc_resist_para[row].resist_cur_max = idata;
			break;
		}

		hwlog_info("ctc_resist_para[%d][%d]=%d\n", row, col, idata);
	}
}

static void direct_charge_parse_temp_para(struct device_node *np,
	struct direct_charge_device *di)
{
	int i;
	int row;
	int col;
	int ret;
	int array_len;
	int idata;
	const char *tmp_string = NULL;

	array_len = of_property_count_strings(np, "temp_para");
	hwlog_info("temp_para array_len=%d\n", array_len);

	if ((array_len <= 0) ||
		(array_len % DC_TEMP_TOTAL != 0) ||
		(array_len > DC_TEMP_LEVEL * DC_TEMP_TOTAL)) {
		hwlog_err("temp_para invalid\n");
		return;
	}

	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np, "temp_para",
			i, &tmp_string);
		if (ret) {
			hwlog_err("temp_para dts read failed\n");
			return;
		}

		ret = kstrtoint(tmp_string, DECIMAL_BASE, &idata);
		if (ret) {
			hwlog_err("get kstrtoint failed\n");
			return;
		}

		row = i / DC_TEMP_TOTAL;
		col = i % DC_TEMP_TOTAL;

		switch (col) {
		case DC_TEMP_MIN:
			di->temp_para[row].temp_min = idata;
			break;

		case DC_TEMP_MAX:
			di->temp_para[row].temp_max = idata;
			break;

		case DC_TEMP_CUR_MAX:
			di->temp_para[row].temp_cur_max = idata;

			if (idata > di->iin_thermal_default)
				di->iin_thermal_default = idata;
			break;
		}

		hwlog_info("temp_para[%d][%d]=%d\n", row, col, idata);
	}

	hwlog_info("iin_thermal_default=%d\n", di->iin_thermal_default);
}

static int direct_charge_parse_stage_jump_para(struct device_node *np,
	struct direct_charge_device *di)
{
	int i;
	int ret;
	int array_len;
	int idata;
	const char *tmp_string = NULL;

	for (i = 0; i < 2 * DC_VOLT_LEVEL; ++i)
		di->stage_need_to_jump[i] = -1;

	array_len = of_property_count_strings(np, "stage_need_to_jump");
	hwlog_info("stage_need_to_jump array_len=%d\n", array_len);

	if (array_len <= 0) {
		hwlog_err("stage_need_to_jump invalid\n");
		return -EINVAL;
	}

	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np, "stage_need_to_jump",
			i, &tmp_string);
		if (ret) {
			hwlog_err("stage_need_to_jump dts read failed\n");
			return -EINVAL;
		}

		ret = kstrtoint(tmp_string, DECIMAL_BASE, &idata);
		if (ret) {
			hwlog_err("get kstrtoint failed\n");
			return -EINVAL;
		}

		di->stage_need_to_jump[i] = idata;

		hwlog_info("stage_need_to_jump[%d]=%d\n", i, idata);
	}

	return 0;
}

static int direct_charge_parse_basic_para(struct device_node *np,
	struct direct_charge_device *di)
{
	int ret;

	ret = of_property_read_u32(np, "need_wired_sw_off",
		&g_dc_need_wired_sw_off);
	if (ret) {
		hwlog_err("need_wired_sw_off dts read failed\n");
		g_dc_need_wired_sw_off = 1;
	}
	hwlog_info("need_wired_sw_off=%d\n", g_dc_need_wired_sw_off);

	ret = of_property_read_u32(np, "adaptor_detect_by_voltage",
		&(di->adaptor_detect_by_voltage));
	if (ret) {
		hwlog_err("adaptor_detect_by_voltage dts read failed\n");
		di->adaptor_detect_by_voltage = 0;
	}
	hwlog_info("adaptor_detect_by_voltage=%d\n",
		di->adaptor_detect_by_voltage);

	ret = of_property_read_u32(np, "dc_volt_ratio", &(di->dc_volt_ratio));
	if (ret) {
		hwlog_err("dc_volt_ratio dts read failed\n");
		di->dc_volt_ratio = 1;
	}
	hwlog_info("dc_volt_ratio=%d\n", di->dc_volt_ratio);

	ret = of_property_read_u32(np, "init_adapter_vset",
		&(di->init_adapter_vset));
	if (ret) {
		hwlog_err("init_adapter_vset dts read failed\n");
		di->init_adapter_vset = 4400; /* default is 4400mv */
	}
	hwlog_info("init_adapter_vset=%d\n", di->init_adapter_vset);

	ret = of_property_read_u32(np, "init_delt_vset", &(di->init_delt_vset));
	if (ret) {
		hwlog_err("init_delt_vset dts read failed\n");
		di->init_delt_vset = 300; /* default is 300mv */
	}
	hwlog_info("init_delt_vset=%d\n", di->init_delt_vset);

	ret = of_property_read_u32(np, "scp_work_on_charger",
		&(di->scp_work_on_charger));
	if (ret) {
		hwlog_err("scp_work_on_charger dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("scp_work_on_charger=%d\n", di->scp_work_on_charger);

	ret = of_property_read_u32(np, "standard_cable_full_path_res_max",
		&(di->standard_cable_full_path_res_max));
	if (ret) {
		hwlog_err("standard_cable_full_path_res_max dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("standard_cable_full_path_res_max=%d\n",
		di->standard_cable_full_path_res_max);

	ret = of_property_read_u32(np, "max_current_for_none_standard_cable",
		&(di->max_current_for_none_standard_cable));
	if (ret) {
		hwlog_err("max_current_for_none_standard_cable dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("max_current_for_none_standard_cable=%d\n",
		di->max_current_for_none_standard_cable);

	ret = of_property_read_u32(np, "max_current_for_ctc_cable",
		&(di->max_current_for_ctc_cable));
	if (ret) {
		hwlog_err("max_current_for_ctc_cable dts read failed\n");
		di->max_current_for_ctc_cable =
			di->max_current_for_none_standard_cable;
	}
	hwlog_info("max_current_for_ctc_cable=%d\n",
		di->max_current_for_ctc_cable);

	ret = of_property_read_u32(np, "super_ico_current",
		&(di->super_ico_current));
	if (ret) {
		hwlog_err("super_ico_current dts read failed\n");
		di->super_ico_current = DC_SUPER_ICO_CURRENT;
	}
	hwlog_info("super_ico_current=%d\n", di->super_ico_current);

	ret = of_property_read_u32(np, "is_show_ico_first",
		&(di->is_show_ico_first));
	if (ret) {
		hwlog_err("is_show_ico_first dts read failed\n");
		di->is_show_ico_first = 0;
	}
	hwlog_info("is_show_ico_first=%d\n", di->is_show_ico_first);

	ret = of_property_read_u32(np, "use_5A", &(di->use_5a));
	if (ret) {
		hwlog_err("use_5a dts read failed\n");
		di->use_5a = 0;
	}
	hwlog_info("use_5a=%d\n", di->use_5a);

	ret = of_property_read_u32(np, "use_8A", &(di->use_8a));
	if (ret) {
		hwlog_err("use_8a dts read failed\n");
		di->use_8a = 0;
	}
	hwlog_info("use_8a=%d\n", di->use_8a);

	ret = of_property_read_u32(np, "max_tadapt", &(di->max_tadapt));
	if (ret) {
		hwlog_err("max_tadapt dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("max_tadapt=%d\n", di->max_tadapt);

	ret = of_property_read_u32(np, "max_tls", &(di->max_tls));
	if (ret) {
		hwlog_err("max_tls dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("max_tls=%d\n", di->max_tls);

	ret = of_property_read_u32(np, "ibat_abnormal_th",
		&(di->ibat_abnormal_th));
	if (ret) {
		hwlog_err("ibat_abnormal_th dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("ibat_abnormal_th=%d\n", di->ibat_abnormal_th);

	ret = of_property_read_u32(np, "first_cc_stage_timer_in_min",
		&(di->first_cc_stage_timer_in_min));
	if (ret) {
		hwlog_err("first_cc_stage_timer_in_min dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("first_cc_stage_timer_in_min=%d\n",
		di->first_cc_stage_timer_in_min);

	ret = of_property_read_u32(np, "vol_err_th", &(di->vol_err_th));
	if (ret) {
		hwlog_err("vol_err_th dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("vol_err_th=%d\n", di->vol_err_th);

	ret = of_property_read_u32(np, "full_path_res_max",
		&(di->full_path_res_max));
	if (ret) {
		hwlog_err("full_path_res_max dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("full_path_res_max=%d\n", di->full_path_res_max);

	ret = of_property_read_u32(np, "ctc_cable_full_path_res_max",
		&(di->ctc_cable_full_path_res_max));
	if (ret) {
		hwlog_err("ctc_cable_full_path_res_max dts read failed\n");
		di->ctc_cable_full_path_res_max = 320; /* default is 320mohm */
	}
	hwlog_info("ctc_cable_full_path_res_max=%d\n",
		di->ctc_cable_full_path_res_max);

	ret = of_property_read_u32(np, "adaptor_leakage_current_th",
		&(di->adaptor_leakage_current_th));
	if (ret) {
		hwlog_err("adaptor_leakage_current_th dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("adaptor_leakage_current_th=%d\n",
		di->adaptor_leakage_current_th);

	ret = of_property_read_u32(np, "compensate_r", &(di->compensate_r));
	if (ret) {
		hwlog_err("compensate_r dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("compensate_r=%d\n", di->compensate_r);

	ret = of_property_read_u32(np, "cc_protect", &(di->cc_protect));
	if (ret) {
		hwlog_err("cc_protect dts read failed\n");
		di->cc_protect = 0;
	}
	hwlog_info("cc_protect=%d\n", di->cc_protect);

	ret = of_property_read_u32(np, "max_dc_bat_vol", &(di->max_dc_bat_vol));
	if (ret) {
		hwlog_err("max_dc_bat_vol dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("max_dc_bat_vol=%d\n", di->max_dc_bat_vol);

#ifdef CONFIG_SCHARGER_V300
	if (is_hi6523_cv_limit())
		di->max_dc_bat_vol -= HI6523_CV_CUT;

	hwlog_info("max_dc_bat_vol=%d\n", di->max_dc_bat_vol);
#endif /* CONFIG_SCHARGER_V300 */

	ret = of_property_read_u32(np, "min_dc_bat_vol", &(di->min_dc_bat_vol));
	if (ret) {
		hwlog_err("min_dc_bat_vol dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("min_dc_bat_vol=%d\n", di->min_dc_bat_vol);

	ret = of_property_read_u32(np, "max_adaptor_vset",
		&(di->max_adaptor_vset));
	if (ret) {
		hwlog_err("max_adaptor_vset dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("max_adaptor_vset=%d\n", di->max_adaptor_vset);

	ret = of_property_read_u32(np, "charge_control_interval",
		&(di->charge_control_interval));
	if (ret) {
		hwlog_err("charge_control_interval dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("charge_control_interval=%d\n", di->charge_control_interval);

	ret = of_property_read_u32(np, "threshold_caculation_interval",
		&(di->threshold_caculation_interval));
	if (ret) {
		hwlog_err("threshold_caculation_interval dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("threshold_caculation_interval=%d\n",
		di->threshold_caculation_interval);

	ret = of_property_read_u32(np, "vstep", &(di->vstep));
	if (ret) {
		hwlog_err("vstep dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("vstep=%d\n", di->vstep);

	ret = of_property_read_u32(np, "delta_err", &(di->delta_err));
	if (ret) {
		hwlog_err("delta_err dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("delta_err=%d\n", di->delta_err);

	ret = of_property_read_u32(np, "cc_cable_detect_enable",
		&(di->cc_cable_detect_enable));
	if (ret) {
		hwlog_err("cc_cable_detect_enable dts read failed\n");
		di->cc_cable_detect_enable = 0;
	}
	hwlog_info("cc_cable_detect_enable=%d\n", di->cc_cable_detect_enable);

	ret = of_property_read_u32(np, "reset_adap_volt_enabled",
		&(di->reset_adap_volt_enabled));
	if (ret) {
		hwlog_err("reset_adap_volt_enabled dts read failed\n");
		di->reset_adap_volt_enabled = 0;
	}
	hwlog_info("reset_adap_volt_enabled=%d\n", di->reset_adap_volt_enabled);

	ret = of_property_read_u32(np, "open_path_vol_select",
		&(di->open_path_vol_select));
	if (ret) {
		hwlog_err("open_path_vol_select dts read failed\n");
		di->open_path_vol_select = 0;
	}
	hwlog_info("open_path_vol_select=%d\n", di->open_path_vol_select);

	return 0;
}

int direct_charge_parse_dts(struct device_node *np,
	struct direct_charge_device *di)
{
	if (!np || !di) {
		hwlog_err("np or di is null\n");
		return -EINVAL;
	}

	if (direct_charge_parse_basic_para(np, di))
		return -EINVAL;

	if (direct_charge_parse_stage_jump_para(np, di))
		return -EINVAL;

	direct_charge_parse_temp_para(np, di);
	direct_charge_parse_non_std_resist_para(np, di);
	direct_charge_parse_std_resist_para(np, di);
	direct_charge_parse_ctc_resist_para(np, di);
	direct_charge_parse_bat_para(np, di);
	direct_charge_parse_group_volt_para(np, di);
	direct_charge_parse_adapter_antifake_para(np, di);
	direct_charge_parse_gain_current_para(np, di);

	return 0;
}
