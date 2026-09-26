/*
 * direct_charger_adapte.c
 *
 * adapter operate for direct charge
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

#include <huawei_platform/power/direct_charger/direct_charger.h>
#include <huawei_platform/power/battery_voltage.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_auth.h>

#define HWLOG_TAG direct_charge_adapter
HWLOG_REGIST();

static enum adapter_protocol_type g_prot = ADAPTER_PROTOCOL_SCP;
struct dc_pps_ops *g_pps_ops;

/* protocol power supply for direct charge */
int dc_pps_ops_register(struct dc_pps_ops *ops)
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

/* control power supply for adapter protocol communication */
void dc_adapter_protocol_power_supply(int enable)
{
	int ret;

	if (!g_pps_ops || !g_pps_ops->enable) {
		hwlog_err("pps_ops or power_supply_enable is null\n");
		return;
	}

	ret = g_pps_ops->enable(enable);
	if (ret) {
		hwlog_err("power supply fail, enable=%d\n", enable);
		return;
	}

	hwlog_info("power supply ok, enable=%d\n", enable);
}

int dc_init_adapter(void)
{
	struct adapter_init_data aid;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	aid.scp_mode_enable = 1;
	aid.vset_boundary = l_di->max_adapter_vset;
	aid.iset_boundary = l_di->max_adapter_iset;
	aid.init_voltage = l_di->init_adapter_vset;
	aid.watchdog_timer = l_di->adapter_watchdog_time;

	return adapter_set_init_data(g_prot, &aid);
}

int dc_reset_operate(int type)
{
	switch (type) {
	case DC_RESET_ADAPTER:
		hwlog_info("soft reset adapter\n");
		return adapter_soft_reset_slave(g_prot);
	case DC_RESET_MASTER:
		hwlog_info("soft reset master\n");
		return adapter_soft_reset_master(g_prot);
	default:
		hwlog_info("reset operate invalid\n");
		return -1;
	}
}

int dc_get_adapter_type(void)
{
	int adapter_type = ADAPTER_TYPE_UNKNOWN;

	adapter_get_adp_type(g_prot, &adapter_type);

	return adapter_type;
}

int dc_get_adapter_support_mode(void)
{
	int adapter_mode = ADAPTER_SUPPORT_UNDEFINED;

	adapter_get_support_mode(g_prot, &adapter_mode);

	return adapter_mode;
}

int dc_get_adapter_port_leakage_current(void)
{
	int leak_current = 0;

	adapter_get_port_leakage_current_flag(g_prot, &leak_current);

	return leak_current;
}

int dc_get_adapter_voltage(int *value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	if (direct_charge_get_stop_charging_flag())
		return -1;

	if (adapter_get_output_voltage(g_prot, value)) {
		dc_fill_eh_buf(l_di->dsm_buff, sizeof(l_di->dsm_buff),
			DC_EH_GET_VAPT, NULL);
		direct_charge_set_stop_charging_flag(1);
		*value = 0;
		hwlog_err("get adapter output voltage fail\n");
		return -1;
	}

	return 0;
}

int dc_get_adapter_max_voltage(int *value)
{
	return adapter_get_max_voltage(g_prot, value);
}

int dc_get_adapter_min_voltage(int *value)
{
	return adapter_get_min_voltage(g_prot, value);
}

int dc_get_adapter_current(int *value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	if (direct_charge_get_stop_charging_flag())
		return -1;

	if (l_di->adaptor_vendor_id == ADAPTER_CHIP_IWATT)
		return direct_charge_get_device_ibus(value);

	if (adapter_get_output_current(g_prot, value)) {
		dc_fill_eh_buf(l_di->dsm_buff, sizeof(l_di->dsm_buff),
			DC_EH_GET_IAPT, NULL);
		direct_charge_set_stop_charging_flag(1);
		*value = 0;
		hwlog_err("get adapter output current fail\n");
		return -1;
	}

	return 0;
}

int dc_get_adapter_current_set(int *value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	if (direct_charge_get_stop_charging_flag())
		return -1;

	if (adapter_get_output_current_set(g_prot, value)) {
		dc_fill_eh_buf(l_di->dsm_buff, sizeof(l_di->dsm_buff),
			DC_EH_GET_APT_CURR_SET, NULL);
		direct_charge_set_stop_charging_flag(1);
		*value = 0;
		hwlog_err("get adapter setting current fail\n");
		return -1;
	}

	return 0;
}

/* get the maximum current allowed by adapter at specified voltage */
static int dc_adapter_volt_handler(int val,
	struct direct_charge_adp_cur_para *para)
{
	int i;

	if (!para)
		return 0;

	for (i = 0; i < DC_ADP_CUR_LEVEL; ++i) {
		if ((val > para[i].vol_min) && (val <= para[i].vol_max))
			return para[i].cur_th;
	}

	return 0;
}

static int dc_get_adapter_max_cur_by_config(int val,
	struct direct_charge_device *di)
{
	int adapter_type = dc_get_adapter_type();

	switch (adapter_type) {
	case ADAPTER_TYPE_10V2P25A:
		return dc_adapter_volt_handler(val,
			di->adp_10v2p25a);
	case ADAPTER_TYPE_10V2P25A_CAR:
		return dc_adapter_volt_handler(val,
			di->adp_10v2p25a_car);
	case ADAPTER_TYPE_QTR_A_10V2P25A:
		return dc_adapter_volt_handler(val,
			di->adp_qtr_a_10v2p25a);
	case ADAPTER_TYPE_QTR_C_20V3A:
		return dc_adapter_volt_handler(val,
			di->adp_qtr_c_20v3a);
	case ADAPTER_TYPE_10V4A:
		return dc_adapter_volt_handler(val,
			di->adp_10v4a);
	default:
		return 0;
	}
}

static int dc_get_adapter_max_cur_by_power_curve(int val,
	struct direct_charge_device *di)
{
	int i;

	for (i = 0; i < DC_ADP_PC_LEVEL; i++) {
		if (val <= di->adp_pwr_curve[i].volt)
			return di->adp_pwr_curve[i].cur;
	}

	return 0;
}

static int dc_get_adapter_max_cur_by_reg(int *value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	if (direct_charge_get_stop_charging_flag())
		return -1;

	if (adapter_get_max_current(g_prot, value)) {
		dc_fill_eh_buf(l_di->dsm_buff, sizeof(l_di->dsm_buff),
			DC_EH_GET_APT_MAX_CURR, NULL);
		direct_charge_set_stop_charging_flag(1);
		*value = 0;
		hwlog_err("get adapter max current fail\n");
		return -1;
	}

	return 0;
}

int dc_get_power_drop_current(int *value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	if (direct_charge_get_stop_charging_flag())
		return -1;

	if (adapter_get_power_drop_current(g_prot, value)) {
		dc_fill_eh_buf(l_di->dsm_buff, sizeof(l_di->dsm_buff),
			DC_EH_GET_APT_PWR_DROP_CURR, NULL);
		direct_charge_set_stop_charging_flag(1);
		*value = 0;
		hwlog_err("get adapter power drop current fail\n");
		return -1;
	}

	return 0;
}

void dc_reset_adapter_power_curve(void *p)
{
	struct direct_charge_device *l_di = (struct direct_charge_device *)p;

	if (!l_di)
		return;

	l_di->adapter_power_curve_flag = false;
	memset(l_di->adp_pwr_curve, 0,
		sizeof(struct direct_charge_adp_pc_para) * DC_ADP_PC_LEVEL);
}

static int dc_get_power_curve_num(int *num)
{
	if (!num)
		return -1;

	if (direct_charge_get_stop_charging_flag())
		return -1;

	return adapter_get_power_curve_num(g_prot, num);
}

static int dc_get_power_curve(int *val, unsigned int size)
{
	if (!val)
		return -1;

	if (direct_charge_get_stop_charging_flag())
		return -1;

	return adapter_get_power_curve(g_prot, val, size);
}

int dc_get_adapter_power_curve(void)
{
	int i;
	int num = 0;
	int value[DC_ADP_PC_LEVEL * DC_ADP_PC_PARA_SIZE] = { 0 };
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	dc_reset_adapter_power_curve(l_di);
	if (dc_get_power_curve_num(&num))
		return -1;

	if ((num <= 0) || (num > DC_ADP_PC_LEVEL))
		return -1;

	if (dc_get_power_curve(value, num * DC_ADP_PC_PARA_SIZE))
		return -1;

	for (i = 0; i < num; i++) {
		l_di->adp_pwr_curve[i].volt = value[i * DC_ADP_PC_PARA_SIZE];
		l_di->adp_pwr_curve[i].cur = value[i * DC_ADP_PC_PARA_SIZE + 1];
	}

	l_di->adapter_power_curve_flag = true;
	for (i = 0; i < DC_ADP_PC_LEVEL; i++)
		hwlog_info("adp_pwr_curve[%d].volt=%d, adp_pwr_curve[%d].cur=%d\n",
			i, l_di->adp_pwr_curve[i].volt,
			i, l_di->adp_pwr_curve[i].cur);
	return 0;
}

int dc_get_adapter_max_current(int value)
{
	int max_cur = 0;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return 0;

	if (l_di->adp_antifake_execute_enable == 0 &&
		l_di->adp_antifake_failed_cnt >= ADP_ANTIFAKE_CHECK_THLD) {
		hwlog_info("antifake check failed, set max current to %d\n", ADP_ANTIFAKE_FAIL_MAX_CURRENT);
		return ADP_ANTIFAKE_FAIL_MAX_CURRENT;
	}

	if (l_di->adapter_power_curve_flag)
		max_cur = dc_get_adapter_max_cur_by_power_curve(value, l_di);
	if (max_cur <= 0)
		max_cur = dc_get_adapter_max_cur_by_config(value, l_di);
	if (max_cur <= 0)
		dc_get_adapter_max_cur_by_reg(&max_cur);

	return max_cur;
}

int dc_get_adapter_temp(int *value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	if (direct_charge_get_stop_charging_flag())
		return -1;

	if (adapter_get_inside_temp(g_prot, value)) {
		dc_fill_eh_buf(l_di->dsm_buff, sizeof(l_di->dsm_buff),
			DC_EH_GET_APT_TEMP, NULL);
		direct_charge_set_stop_charging_flag(1);
		*value = 0;
		hwlog_err("get adapter temp fail\n");
		return -1;
	}

	return 0;
}

int dc_get_protocol_register_state(void)
{
	if (adapter_get_protocol_register_state(g_prot)) {
		hwlog_err("adapter protocol not ready\n");
		return -1;
	}

	direct_charge_set_stage_status(DC_STAGE_ADAPTER_DETECT);
	return 0;
}

int dc_set_adapter_voltage(int value)
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

	if (adapter_set_output_voltage(g_prot, l_value)) {
		dc_fill_eh_buf(l_di->dsm_buff, sizeof(l_di->dsm_buff),
			DC_EH_SET_APT_VOLT, NULL);
		direct_charge_set_stop_charging_flag(1);
		hwlog_err("set adapter voltage fail\n");
		return -1;
	}

	return 0;
}

int dc_set_adapter_current(int value)
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

	if (adapter_set_output_current(g_prot, l_value)) {
		dc_fill_eh_buf(l_di->dsm_buff, sizeof(l_di->dsm_buff),
			DC_EH_SET_APT_CURR, NULL);
		direct_charge_set_stop_charging_flag(1);
		hwlog_err("set adapter current fail\n");
		return -1;
	}

	return 0;
}

int dc_set_adapter_output_enable(int enable)
{
	int ret, i;
	int retry = 3; /* retry 3 times */
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	hwlog_info("set adapter_output_enable=%d\n", enable);

	for (i = 0; i < retry; i++) {
		ret = adapter_set_output_enable(g_prot, enable);
		if (!ret)
			break;
	}

	return ret;
}

void dc_set_adapter_default_param(void)
{
	adapter_set_default_param(g_prot);
}

int dc_set_adapter_default(void)
{
	return adapter_set_default_state(g_prot);
}

int dc_update_adapter_info(void)
{
	int ret;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	ret = adapter_get_device_info(g_prot);
	if (ret) {
		hwlog_err("get adapter info fail\n");
		return -1;
	}
	adapter_show_device_info(g_prot);

	ret = adapter_get_chip_vendor_id(g_prot,
		&l_di->adaptor_vendor_id);
	if (ret) {
		hwlog_err("get adapter vendor id fail\n");
		return -1;
	}

	return 0;
}

unsigned int dc_update_adapter_support_mode(void)
{
	unsigned int adp_mode = ADAPTER_SUPPORT_SCP_B_LVC +
		ADAPTER_SUPPORT_SCP_B_SC;

	adapter_update_adapter_support_mode(g_prot, &adp_mode);

	return adp_mode;
}

int dc_check_adapter_support_mode(int *mode)
{
	return adapter_detect_adapter_support_mode(g_prot, mode);
}

bool dc_get_adapter_antifake_result(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return true;

	if (!l_di->adp_antifake_execute_enable)
		return true;

	return l_di->adp_antifake_result != ADAPTER_ANTIFAKE_FAIL;
}

int dc_get_adapter_antifake_failed_cnt(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return 0;

	return l_di->adp_antifake_failed_cnt;
}

static void dc_report_antifake_result(struct direct_charge_device *di,
	int res, int cur, int pwr)
{
	int ret;
	int len;
	int vendor_id = 0;
	int serial_num = 0;
	int adp_type = ADAPTER_TYPE_UNKNOWN;
	unsigned int adp_mode;
	int antifake_state;
	char auth_buf[POWER_EVENT_NOTIFY_SIZE] = { 0 };
	struct power_event_notify_data n_data;

	if (res)
		antifake_state = ADAPTER_ANTIFAKE_FAIL;
	else
		antifake_state = ADAPTER_ANTIFAKE_SUCC;

	if (antifake_state == di->adp_antifake_result)
		return;

	di->adp_antifake_result = antifake_state;

	if (charge_get_charger_online() == 0) {
		hwlog_info("charger has been removed, no need report event\n");
		return;
	}

	ret = adapter_get_chip_vendor_id(g_prot, &vendor_id);
	if (ret)
		hwlog_info("get vendor id fail\n");

	ret = adapter_get_chip_serial_num(g_prot, &serial_num);
	if (ret)
		hwlog_info("get serial num fail\n");

	ret = adapter_get_adp_type(g_prot, &adp_type);
	if (ret)
		hwlog_info("get adapter type fail\n");

	adp_mode = direct_charge_detect_adapter_support_mode();

	len = snprintf(auth_buf, POWER_EVENT_NOTIFY_SIZE - 1,
		"BMS_EVT=EVT_ADAPTER_AUTH_UPDATE@vendor_id:%d,serial_no:%d,protocol:scp,adp_type:%d,adp_mode:%u,max_cur:%d,max_pwr:%d,auth_state:%d\n",
		vendor_id, serial_num, adp_type, adp_mode, cur, pwr, antifake_state);
	if (len <= 0)
		return;

	n_data.event_len = len;
	n_data.event = auth_buf;
	if (di->adp_antifake_execute_enable)
		power_event_report_uevent(&n_data);
	else
		hwlog_info("adapter antifake result:%s\n", auth_buf);

	if (antifake_state == ADAPTER_ANTIFAKE_FAIL)
		power_dsm_report_dmd(POWER_DSM_BATTERY, ERROR_NON_STANDARD_CHARGER_PLUGGED, auth_buf);
}

int dc_check_adapter_antifake(void)
{
	int ret;
	int max_cur;
	int max_pwr;
	int bat_vol = hw_battery_get_series_num() * BAT_RATED_VOLT;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	if (!l_di->adp_antifake_enable)
		return 0;

	if (direct_charge_is_priority_inversion()) {
		hwlog_info("antifake already checked\n");
		return 0;
	}

	max_cur = dc_get_adapter_max_current(
		bat_vol * l_di->dc_volt_ratio);
	if (max_cur == 0)
		return -1;

	max_pwr = max_cur * BAT_RATED_VOLT * l_di->dc_volt_ratio *
		hw_battery_get_series_num() / 1000; /* unit: 1000mw */
	hwlog_info("max_cur=%d, max_pwr=%d\n", max_cur, max_pwr);
	if (max_pwr <= POWER_TH_IGNORE_ANTIFAKE)
		return 0;

	dc_auth_clean_hash_data();

	ret = adapter_auth_encrypt_start(g_prot,
		l_di->adp_antifake_key_index,
		dc_auth_get_hash_data_header(), dc_auth_get_hash_data_size());
	if (ret)
		goto end_check;

	ret = dc_auth_wait_completion();

end_check:
	dc_auth_clean_hash_data();
	ret += adapter_auth_encrypt_release(g_prot);
	dc_report_antifake_result(l_di, ret, max_cur, max_pwr);
	return ret;
}

bool dc_is_undetach_cable(void)
{
	return adapter_is_undetach_cable(g_prot);
}
