/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2020. All rights reserved.
 * Description: this file provide interface to get the battery state such as
 *          capacity, voltage, current, temperature
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/power_supply.h>
#include <linux/power/hisi/coul/coul_drv.h>
#include <linux/power/hisi/bci_battery.h>

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include <coul_drv_test.h>

#define coul_drv_debug(fmt, args...) pr_debug("[coul_drv]" fmt, ## args)
#define coul_drv_info(fmt, args...) pr_info("[coul_drv]" fmt, ## args)
#define coul_drv_warn(fmt, args...) pr_warn("[coul_drv]" fmt, ## args)
#define coul_drv_err(fmt, args...) pr_err("[coul_drv]" fmt, ## args)

static int g_coul_drv_init;
static struct mutex g_coul_drv_lock;
#define coul_drv_lock_f() do { \
	if (!g_coul_drv_init) return; \
	mutex_lock(&g_coul_drv_lock); \
} while (0)
#define coul_drv_unlock_f() mutex_unlock(&g_coul_drv_lock)
static struct coulometer_ops *g_coul_ops;
static enum COULOMETER_TYPE g_coulometer_type = COUL_UNKNOW;

enum COULOMETER_TYPE coul_drv_coulometer_type(void)
{
	return g_coulometer_type;
}

int coul_drv_is_coul_ready(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* make sure coul moudule have registered */
	if (g_coul_ops == NULL)
		return 0;

	/* execute the operation of coul module */
	if (ops && ops->is_coul_ready)
		return ops->is_coul_ready();

	return 0;
}

/* check whether fcc is debounce */
int coul_drv_is_fcc_debounce(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->coul_is_fcc_debounce)
		return ops->coul_is_fcc_debounce();
	return 0;
}

/* get battery polar avg */
int coul_drv_get_polar_avg(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->get_polar_avg)
		return ops->get_polar_avg();

	return 0;
}

int coul_drv_get_polar_peak(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->get_polar_peak)
		return ops->get_polar_peak();

	return 0;
}

int coul_drv_battery_get_qmax(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->get_qmax)
		return ops->get_qmax();

	return 0;
}

int coul_drv_battery_update_basp_policy(unsigned int level,
	unsigned int nondc_volt_dec)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->update_basp_policy)
		return ops->update_basp_policy(level,
					nondc_volt_dec);

	return -EPERM;
}

int coul_drv_battery_get_recrod_fcc(unsigned int size, unsigned int *records)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->get_record_fcc)
		return ops->get_record_fcc(size, records);

	return -EPERM;
}

/*  set the flag for high or low temperature test */
int coul_drv_set_hltherm_flag(int temp_flag)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->set_hltherm_flag)
		return ops->set_hltherm_flag(temp_flag);
	return 0;
}

int coul_drv_get_hltherm_flag(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->get_hltherm_flag)
		return ops->get_hltherm_flag();

	return 0;
}

int coul_drv_is_battery_exist(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;
	struct coul_drv_test_info *test_info = get_coul_drv_test_info();

	if (test_info) {
		if (test_info->test_start_flag & 0x0001) {
			coul_drv_info("_battery exist status is %d\n", test_info->input_batt_exist);
			return test_info->input_batt_exist;
		}
	}
	/* execute the operation of coul module */
	if (ops && ops->is_battery_exist)
		return ops->is_battery_exist();

	return 0;
}

/* check whether remaining capacity of battery reach the low power threshold */
int coul_drv_is_battery_reach_threshold(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->is_battery_reach_threshold)
		return ops->is_battery_reach_threshold();

	return 0;
}

char *coul_drv_battery_brand(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_brand)
		return ops->battery_brand();

	return "error";
}

int coul_drv_battery_id_voltage(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_id_voltage)
		return ops->battery_id_voltage();

	return -EPERM;
}

int coul_drv_battery_voltage(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;
	struct coul_drv_test_info *test_info = get_coul_drv_test_info();

	if (test_info) {
		if (test_info->test_start_flag & 0x0004) {
			coul_drv_info("the battery voltage is %d\n", test_info->input_batt_volt);
			return test_info->input_batt_volt;
		}
	}
	/* execute the operation of coul module */
	if (ops && ops->battery_voltage)
		return ops->battery_voltage();

	return -EPERM;
}

int coul_drv_battery_voltage_uv(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_voltage_uv)
		return ops->battery_voltage_uv();

	return -EPERM;
}

int coul_drv_battery_current(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;
	struct coul_drv_test_info *test_info = get_coul_drv_test_info();

	if (test_info) {
		if (test_info->test_start_flag & 0x0008) {
			coul_drv_info("the input batt cur is %d\n", test_info->input_batt_cur);
			return test_info->input_batt_cur;
		}
	}
	/* execute the operation of coul module */
	if (ops && ops->battery_current)
		return ops->battery_current();

	return 0;
}
int coul_drv_get_calibration_status(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->get_coul_calibration_status)
		return ops->get_coul_calibration_status();

	return 0;
}

int coul_drv_battery_resistance(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_resistance)
		return ops->battery_resistance();

	return 0;
}

int coul_drv_fifo_avg_current(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->fifo_avg_current)
		return ops->fifo_avg_current();

	return 0;
}

int coul_drv_battery_current_avg(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_current_avg)
		return ops->battery_current_avg();

	return 0;
}

int coul_drv_battery_unfiltered_capacity(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_unfiltered_capacity)
		return ops->battery_unfiltered_capacity();

	return -EPERM;
}

int coul_drv_battery_capacity(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;
	struct coul_drv_test_info *test_info = get_coul_drv_test_info();

	if (test_info) {
		if (test_info->test_start_flag & 0x0010) {
			coul_drv_info("input batt cap is %d\n", test_info->input_batt_capacity);
			return test_info->input_batt_capacity;
		}
	}
	/* execute the operation of coul module */
	if (ops && ops->battery_capacity)
		return ops->battery_capacity();

	return -EPERM;
}

int coul_drv_battery_temperature(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;
	struct coul_drv_test_info *test_info = get_coul_drv_test_info();

	if (test_info) {
		if (test_info->test_start_flag & 0x0020) {
			coul_drv_info("input batt temp is %d\n", test_info->input_batt_temp);
			return test_info->input_batt_temp;
		}
	}
	/* execute the operation of coul module */
	if (ops && ops->battery_temperature)
		return ops->battery_temperature();

	return -EPERM;
}

int coul_drv_battery_temperature_for_charger(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;
	struct coul_drv_test_info *test_info = get_coul_drv_test_info();

	if (test_info) {
		if (test_info->test_start_flag & 0x0020) {
			coul_drv_info("input batt temp is %d\n", test_info->input_batt_temp);
			return test_info->input_batt_temp;
		}
	}
	/* execute the operation of coul module */
	if (ops && ops->battery_temperature_for_charger)
		return ops->battery_temperature_for_charger();

	return -EPERM;
}

/* get raw battery temperature */
int coul_drv_battery_temperature_raw(int *raw_temp)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->get_battery_temperature_raw && raw_temp) {
		*raw_temp = ops->get_battery_temperature_raw();
		return 0;
	}

	return -EPERM;
}

/* get raw battery temperature  in permille */
int coul_drv_battery_temperature_permille(int *raw_temp)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->get_battery_temp_permille && raw_temp) {
		*raw_temp = ops->get_battery_temp_permille();
		return 0;
	}

	return -EPERM;
}

int coul_drv_coulobmeter_cur_calibration(int *cur_offset_a, int *cur_offset_b)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->get_c_calibration) {
		ops->get_c_calibration(cur_offset_a, cur_offset_b);
		return 0;
	}

	return -EPERM;
}

/* monitor soc if vary too fast */
int coul_drv_battery_soc_vary_flag(int monitor_flag, int *deta_soc)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->get_soc_vary_flag)
		return ops->get_soc_vary_flag(monitor_flag, deta_soc);

	return -EPERM;
}

int coul_drv_battery_rm(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_rm)
		return ops->battery_rm();

	return -EPERM;
}

int coul_drv_battery_fcc(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;
	struct coul_drv_test_info *test_info = get_coul_drv_test_info();

	if (test_info) {
		if (test_info->test_start_flag & 0x0040) {
			coul_drv_info("input batt fcc is %d\n",
				test_info->input_batt_fcc);
			return test_info->input_batt_fcc;
		}
	}
	/* execute the operation of coul module */
	if (ops && ops->battery_fcc)
		return ops->battery_fcc();

	return -EPERM;
}

int coul_drv_battery_fcc_design(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_fcc_design)
		return ops->battery_fcc_design();

	return -EPERM;
}

int coul_drv_battery_tte(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_tte)
		return ops->battery_tte();

	return -EPERM;
}

int coul_drv_battery_ttf(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_ttf)
		return ops->battery_ttf();

	return -EPERM;
}

int coul_drv_battery_health(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_health)
		return ops->battery_health();

	return 0;
}

int coul_drv_battery_capacity_level(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_capacity_level)
		return ops->battery_capacity_level();

	return POWER_SUPPLY_CAPACITY_LEVEL_UNKNOWN;
}

int coul_drv_battery_technology(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_technology)
		return ops->battery_technology();

	return POWER_SUPPLY_TECHNOLOGY_LIPO;
}

struct chrg_para_lut *coul_drv_battery_charge_params(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_charge_params)
		return ops->battery_charge_params();

	return NULL;
}

int coul_drv_battery_ifull(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_ifull)
		return ops->battery_ifull();

	return -EPERM;
}

int coul_drv_battery_vbat_max(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_vbat_max)
		return ops->battery_vbat_max();

	return -EPERM;
}

int coul_drv_battery_get_limit_fcc(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->get_battery_limit_fcc)
		return ops->get_battery_limit_fcc();

	return -EPERM;
}

int coul_drv_battery_cycle_count(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_cycle_count)
		return ops->battery_cycle_count();

	return -EPERM;
}

int coul_drv_battery_cc(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_cc)
		return ops->battery_cc() / PERMILLAGE;

	return -EPERM;
}

int coul_drv_battery_cc_uah(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_cc)
		return ops->battery_cc();

	return -EPERM;
}

int coul_drv_power_supply_voltage(void)
{
	return DEFAULT_POWER_SUPPLY_VOLTAGE;
}

void coul_drv_charger_event_rcv(unsigned int event)
{
	struct blocking_notifier_head *notifier_list = NULL;
	struct coul_drv_test_info *test_info = NULL;
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	get_notifier_list(&notifier_list);

	coul_drv_lock_f();
	test_info = get_coul_drv_test_info();
	if (test_info) {
		if (test_info->test_start_flag & 0x0080) {
			coul_drv_info("input event is %d\n",
				test_info->input_event);
			event = (unsigned int)test_info->input_event;
		}
	}
	/* execute the operation of coul module */
	if (ops && ops->charger_event_rcv) {
		coul_drv_info("charger event = 0x%x\n", (int)event);
		ops->charger_event_rcv(event);
	}

	blocking_notifier_call_chain(notifier_list, event, NULL);

	coul_drv_unlock_f();
}

int coul_drv_low_temp_opt(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->coul_low_temp_opt)
		return ops->coul_low_temp_opt();

	return 0;
}

int coul_drv_battery_fifo_curr(unsigned int index)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	if (ops && ops->battery_fifo_curr)
		return ops->battery_fifo_curr(index);

	return 0;
}

int coul_drv_battery_fifo_vol(unsigned int index)
{
	struct coulometer_ops *ops = g_coul_ops;

	if (ops && ops->battery_fifo_vol)
		return ops->battery_fifo_vol(index);

	return 0;
}

int coul_drv_battery_rpcb(void)
{
	struct coulometer_ops *ops = g_coul_ops;

	if (ops && ops->battery_rpcb)
		return ops->battery_rpcb();

	return 0;
}

/*
 * users need to check ocv update conditions
 * can not be called on resume and charge done
 */
void coul_drv_force_ocv(void)
{
	struct coulometer_ops *ops = g_coul_ops;

	coul_drv_lock_f();
	if (ops && ops->force_ocv)
		ops->force_ocv();

	coul_drv_unlock_f();
}

int coul_drv_battery_fifo_depth(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_fifo_depth)
		return ops->battery_fifo_depth();

	return 0;
}

int coul_drv_battery_ufcapacity_tenth(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_ufcapacity_tenth)
		return ops->battery_ufcapacity_tenth();

	return 0;
}

int coul_drv_battery_removed_before_boot(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_removed_before_boot)
		return ops->battery_removed_before_boot();

	return -1;
}

int coul_drv_coul_ops_register(struct coulometer_ops *coul_ops,
	enum COULOMETER_TYPE coul_type)
{
	if (g_coul_ops) {
		coul_drv_err("coul ops have registered already\n");
		return -EBUSY;
	}

	g_coul_ops = coul_ops;
	g_coulometer_type = coul_type;

	return 0;
}

int coul_drv_convert_regval2ua(unsigned int reg_val)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->convert_regval2ua)
		return ops->convert_regval2ua(reg_val);

	return -EPERM;
}

int coul_drv_convert_regval2uv(unsigned int reg_val)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->convert_regval2uv)
		return ops->convert_regval2uv(reg_val);

	return -EPERM;
}

int coul_drv_convert_regval2temp(unsigned int reg_val)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->convert_regval2temp)
		return ops->convert_regval2temp(reg_val);

	return -EPERM;
}

int coul_drv_convert_mv2regval(int vol_mv)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->convert_mv2regval)
		return ops->convert_mv2regval(vol_mv);

	return -EPERM;
}

int coul_drv_cal_uah_by_ocv(int ocv_uv, int *ocv_soc_uah)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->cal_uah_by_ocv)
		return ops->cal_uah_by_ocv(ocv_uv, ocv_soc_uah);

	return -EPERM;
}

int coul_drv_get_ocv_by_soc(int temp, int soc)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->get_ocv_by_soc)
		return ops->get_ocv_by_soc(temp, soc);

	return -EPERM;
}

int coul_drv_get_soc_by_ocv(int temp, int ocv)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->get_soc_by_ocv)
		return ops->get_soc_by_ocv(temp, ocv);

	return -EPERM;
}

int coul_drv_get_ocv(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->get_ocv)
		return ops->get_ocv();

	return -EPERM;
}

int coul_drv_get_last_powerdown_soc(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->get_last_powerdown_soc)
		return ops->get_last_powerdown_soc();

	return -EPERM;
}

int coul_drv_convert_temp_to_adc(int temp)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->convert_temp_to_adc)
		return ops->convert_temp_to_adc(temp);

	return -EPERM;
}

int coul_drv_coul_chip_temperature(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->chip_temperature)
		return ops->chip_temperature();

	return INVALID_TEMP;
}

int coul_drv_interpolate_pc(int ocv)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->coul_interpolate_pc)
		return ops->coul_interpolate_pc(ocv);

	return 0;
}

int coul_drv_get_desired_charging_current(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->get_desired_charging_current)
		return ops->get_desired_charging_current();

	return 0;
}

int coul_drv_get_desired_charging_voltage(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->get_desired_charging_voltage)
		return ops->get_desired_charging_voltage();

	return 0;
}

int coul_drv_is_smart_battery(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->is_smart_battery)
		return ops->is_smart_battery();

	return 0;
}

int coul_drv_is_battery_full_charged(void)
{
	/* declare the local variable of struct coulometer_ops */
	struct coulometer_ops *ops = g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->is_battery_full_charged)
		return ops->is_battery_full_charged();

	return 0;
}

int coul_drv_coul_ops_unregister(struct coulometer_ops *coul_ops)
{
	if (g_coul_ops == coul_ops) {
		g_coul_ops = NULL;
		g_coulometer_type = COUL_UNKNOW;
		return 0;
	}

	return -EINVAL;
}

int __init coul_drv_init(void)
{
	mutex_init(&g_coul_drv_lock);

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	/* detect coul device successful, set the flag as present */

	if (g_coul_ops && g_coul_ops->dev_check) {
		if (g_coul_ops->dev_check() == COUL_IC_GOOD) {
			coul_drv_info("coul ic is good\n");
			set_hw_dev_flag(DEV_I2C_COUL);
		} else {
			coul_drv_err("coul ic is bad\n");
		}
	} else {
		coul_drv_err("ops dev_check is null\n");
	}
#endif
	g_coul_drv_init = 1;
	coul_drv_info("%s\n", __func__);
	return 0;
}

module_init(coul_drv_init);

void __exit coul_drv_exit(void)
{
	mutex_destroy(&g_coul_drv_lock);
}

module_exit(coul_drv_exit);

MODULE_DESCRIPTION("coul module driver");
MODULE_LICENSE("GPL");
