/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2020. All rights reserved.
 * Description: this file provide interface to get the battery state such as
 *          capacity, voltage, current, temperature
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

#ifndef _COUL_DRV_H
#define _COUL_DRV_H

#include <linux/kernel.h>

#define BAT_VOL_3500    3500
#define BAT_VOL_3600    3600
#define BAT_VOL_3700    3700
#define BAT_VOL_3800    3800
#define BAT_VOL_3900    3900

#define BASP_TAG "[BASP] " /* Battery Aging Safe Policy LOG TAG */

#define VTERM_MAX_DEFAULT_MV            4400
#define MAX_BATT_CHARGE_CUR_RATIO       70 /* 0.7C */
#define COUL_IC_GOOD    0
#define COUL_IC_BAD     1

#define COMPENSATION_PARA_LEVEL         8
#define COMPENSATION_THRESHOLD          200 /* 20c */

#define INVALID_TEMP_VAL                (-999)
#define INVALID_VOLTAGE_VAL             (-999)
#define BATTERY_NORMAL_CUTOFF_VOL       3150
#define DEFAULT_RPCB            10000 /* uohm */
#define POLAR_CALC_INTERVAL     200 /* ms */
#define TEMP_POLAR_REPORT       5
#define CURR_AVG_DEFAULT        19999 /* ma */
/* ma = ua/1000 = uas/s/1000 = uah*3600/s/1000 = uah*18/(s*5) */
#define cc_uas2ma(cc, time) (((cc) * 18) / ((time) * 5))

#define PERMILLAGE 1000
#define DEFAULT_POWER_SUPPLY_VOLTAGE 4200

enum COULOMETER_TYPE {
	KCOUL = 0,
	COUL_BQ27510,
	COUL_BQ28Z610,
	COUL_BQ40Z50,
	COUL_UNKNOW,
};

/* ntc_temp_compensation_para */
enum ntc_temp_compensation_para_info {
	NTC_COMPENSATION_PARA_ICHG = 0,
	NTC_COMPENSATION_PARA_VALUE,
	NTC_COMPENSATION_PARA_TOTAL,
};
struct ntc_temp_compensation_para_data {
	int ntc_compensation_ichg;
	int ntc_compensation_value;
};

struct coulometer_ops {
	int (*is_coul_ready)(void);
	int (*is_battery_exist)(void);
	int (*is_battery_reach_threshold)(void);
	char *(*battery_brand)(void);
	int (*battery_id_voltage)(void);
	int (*battery_voltage)(void);
	int (*battery_voltage_uv)(void);
	int (*battery_current)(void);
	int (*battery_resistance)(void);
	int (*fifo_avg_current)(void);
	int (*battery_current_avg)(void);
	int (*battery_unfiltered_capacity)(void);
	int (*battery_capacity)(void);
	int (*battery_temperature)(void);
	int (*chip_temperature)(void);
	int (*battery_temperature_for_charger)(void);
	int (*battery_rm)(void);
	int (*battery_fcc)(void);
	int (*battery_fcc_design)(void);
	int (*battery_tte)(void);
	int (*battery_ttf)(void);
	int (*battery_health)(void);
	int (*battery_capacity_level)(void);
	int (*battery_technology)(void);
	struct chrg_para_lut *(*battery_charge_params)(void);
	int (*battery_vbat_max)(void);
	int (*battery_ifull)(void);
	int (*charger_event_rcv)(unsigned int);
	int (*battery_cycle_count)(void);
	int (*get_battery_limit_fcc)(void);
	int (*coul_is_fcc_debounce)(void);
	int (*set_hltherm_flag)(int);
	int (*get_hltherm_flag)(void);
	int (*dev_check)(void);
	int (*get_soc_vary_flag)(int monitor_flag, int *deta_soc);
	int (*coul_low_temp_opt)(void);
	int (*battery_cc)(void);
	int (*battery_fifo_curr)(unsigned int);
	int (*battery_fifo_vol)(unsigned int);
	int (*battery_rpcb)(void);
	int (*battery_fifo_depth)(void);
	int (*battery_ufcapacity_tenth)(void);
	int (*convert_regval2ua)(unsigned int reg_val);
	int (*convert_regval2uv)(unsigned int reg_val);
	int (*convert_regval2temp)(unsigned int reg_val);
	int (*convert_mv2regval)(int vol_mv);
	int (*cal_uah_by_ocv)(int ocv_uv, int *ocv_soc_uAh);
	int (*convert_temp_to_adc)(int temp);
	int (*get_coul_calibration_status)(void);
	int (*battery_removed_before_boot)(void);
	int (*get_qmax)(void);
	int (*get_ocv_by_soc)(int temp, int soc);
	int (*get_soc_by_ocv)(int temp, int ocv);
	int (*get_ocv)(void);
	int (*get_polar_avg)(void);
	int (*get_polar_peak)(void);
	int (*update_basp_policy)(unsigned int level,
		unsigned int nondc_volt_dec);
	int (*get_record_fcc)(unsigned int size, unsigned int *records);
	int (*get_last_powerdown_soc)(void);
	int (*get_battery_temperature_raw)(void);
	int (*get_battery_temp_permille)(void);
	void (*get_c_calibration)(int *cur_offset_a, int *cur_offset_b);
	void (*force_ocv)(void);
	int (*coul_interpolate_pc)(int ocv);
	int (*get_desired_charging_current)(void);
	int (*get_desired_charging_voltage)(void);
	int (*is_smart_battery)(void);
	int (*is_battery_full_charged)(void);
};

#ifdef CONFIG_COUL_DRV
enum COULOMETER_TYPE coul_drv_coulometer_type(void);
void coul_drv_charger_event_rcv(unsigned int event);
int coul_drv_is_coul_ready(void);
int coul_drv_is_battery_exist(void);
int coul_drv_is_battery_reach_threshold(void);
int coul_drv_battery_voltage(void);
char *coul_drv_battery_brand(void);
int coul_drv_battery_id_voltage(void);
int coul_drv_battery_voltage_uv(void);
int coul_drv_battery_current(void);
int coul_drv_battery_resistance(void);
int coul_drv_get_calibration_status(void);
int coul_drv_fifo_avg_current(void);
int coul_drv_battery_current_avg(void);
int coul_drv_battery_unfiltered_capacity(void);
int coul_drv_battery_capacity(void);
int coul_drv_battery_temperature(void);
int coul_drv_battery_rm(void);
int coul_drv_battery_fcc(void);
int coul_drv_battery_fcc_design(void);
int coul_drv_battery_tte(void);
int coul_drv_battery_ttf(void);
int coul_drv_battery_health(void);
int coul_drv_battery_capacity_level(void);
int coul_drv_battery_technology(void);
struct chrg_para_lut *coul_drv_battery_charge_params(void);
int coul_drv_battery_ifull(void);
int coul_drv_battery_vbat_max(void);
int coul_drv_battery_cycle_count(void);
int coul_drv_battery_get_limit_fcc(void);
int coul_drv_coul_ops_register(struct coulometer_ops *coul_ops,
	enum COULOMETER_TYPE coul_type);
int coul_drv_coul_ops_unregister(struct coulometer_ops *coul_ops);
int coul_drv_power_supply_voltage(void);
int coul_drv_is_fcc_debounce(void);
int coul_drv_set_hltherm_flag(int temp_flag);
int coul_drv_get_hltherm_flag(void);
int coul_drv_battery_soc_vary_flag(int monitor_flag, int *deta_soc);
int coul_drv_battery_temperature_for_charger(void);
int coul_drv_low_temp_opt(void);
int coul_drv_battery_cc(void);
int coul_drv_battery_fifo_depth(void);
int coul_drv_battery_ufcapacity_tenth(void);
int coul_drv_battery_fifo_curr(unsigned int index);
int coul_drv_convert_regval2ua(unsigned int reg_val);
int coul_drv_convert_regval2uv(unsigned int reg_val);
int coul_drv_convert_regval2temp(unsigned int reg_val);
int coul_drv_convert_mv2regval(int vol_mv);
int coul_drv_cal_uah_by_ocv(int ocv_uv, int *ocv_soc_uah);
int coul_drv_convert_temp_to_adc(int temp);
int coul_drv_coul_chip_temperature(void);
int coul_drv_battery_cc_uah(void);
int coul_drv_battery_removed_before_boot(void);
int coul_drv_battery_get_qmax(void);
int coul_drv_battery_temperature_raw(int *raw_temp);
int coul_drv_battery_temperature_permille(int *raw_temp);
int coul_drv_get_ocv_by_soc(int temp, int soc);
int coul_drv_get_soc_by_ocv(int temp, int ocv);
int coul_drv_get_ocv(void);
int coul_drv_get_polar_avg(void);
int coul_drv_get_polar_peak(void);
int coul_drv_battery_update_basp_policy(unsigned int level,
	unsigned int nondc_volt_dec);
int coul_drv_battery_get_recrod_fcc(unsigned int size, unsigned int *records);
int coul_drv_get_last_powerdown_soc(void);
int coul_drv_coulobmeter_cur_calibration(int *cur_offset_a, int *cur_offset_b);
int coul_drv_battery_fifo_vol(unsigned int index);
int coul_drv_battery_rpcb(void);
void coul_drv_force_ocv(void);
int coul_drv_interpolate_pc(int ocv);
int coul_drv_get_desired_charging_current(void);
int coul_drv_get_desired_charging_voltage(void);
int coul_drv_is_smart_battery(void);
int coul_drv_is_battery_full_charged(void);
#else /* CONFIG_COUL_DRV */

#include <linux/power_supply.h>

static inline enum COULOMETER_TYPE coul_drv_coulometer_type(void)
{
	return COUL_UNKNOW;
}

static inline void coul_drv_charger_event_rcv(unsigned int event)
{
	/* do nothing */
}

static inline int coul_drv_is_coul_ready(void)
{
	return 0;
}

static inline int coul_drv_is_battery_exist(void)
{
	return 0;
}

static inline int coul_drv_is_battery_reach_threshold(void)
{
	return 0;
}

static inline int coul_drv_battery_voltage(void)
{
	return -EPERM;
}

static inline char *coul_drv_battery_brand(void)
{
	return "error";
}

static inline int coul_drv_battery_id_voltage(void)
{
	return -EPERM;
}

static inline int coul_drv_battery_voltage_uv(void)
{
	return -EPERM;
}

static inline int coul_drv_battery_current(void)
{
	return 0;
}

static inline int coul_drv_battery_resistance(void)
{
	return 0;
}

static inline int coul_drv_get_calibration_status(void)
{
	return 0;
}

static inline int coul_drv_fifo_avg_current(void)
{
	return 0;
}

static inline int coul_drv_battery_current_avg(void)
{
	return 0;
}

static inline int coul_drv_battery_unfiltered_capacity(void)
{
	return -EPERM;
}

static inline int coul_drv_battery_capacity(void)
{
	return -EPERM;
}

static inline int coul_drv_battery_temperature(void)
{
	return -EPERM;
}

static inline int coul_drv_battery_rm(void)
{
	return -EPERM;
}

static inline int coul_drv_battery_fcc(void)
{
	return -EPERM;
}

static inline int coul_drv_battery_fcc_design(void)
{
	return -EPERM;
}

static inline int coul_drv_battery_tte(void)
{
	return -EPERM;
}

static inline int coul_drv_battery_ttf(void)
{
	return -EPERM;
}

static inline int coul_drv_battery_health(void)
{
	return 0;
}

static inline int coul_drv_battery_capacity_level(void)
{
	return POWER_SUPPLY_CAPACITY_LEVEL_UNKNOWN;
}

static inline int coul_drv_battery_technology(void)
{
	return POWER_SUPPLY_TECHNOLOGY_UNKNOWN;
}

static inline struct chrg_para_lut *coul_drv_battery_charge_params(void)
{
	return NULL;
}

static inline int coul_drv_battery_ifull(void)
{
	return -EPERM;
}

static inline int coul_drv_battery_vbat_max(void)
{
	return -EPERM;
}

static inline int coul_drv_battery_cycle_count(void)
{
	return -EPERM;
}

static inline int coul_drv_battery_get_limit_fcc(void)
{
	return -EPERM;
}

static inline int coul_drv_coul_ops_register(struct coulometer_ops *coul_ops,
	enum COULOMETER_TYPE coul_type)
{
	return 0;
}

static inline int coul_drv_coul_ops_unregister(struct coulometer_ops *coul_ops)
{
	return 0;
}

static inline int coul_drv_power_supply_voltage(void)
{
	return DEFAULT_POWER_SUPPLY_VOLTAGE;
}

static inline int coul_drv_is_fcc_debounce(void)
{
	return 0;
}

static inline int coul_drv_set_hltherm_flag(int temp_flag)
{
	return 0;
}

static inline int coul_drv_get_hltherm_flag(void)
{
	return 0;
}

static inline int coul_drv_battery_soc_vary_flag(int monitor_flag, int *deta_soc)
{
	return -EPERM;
}

static inline int coul_drv_battery_temperature_for_charger(void)
{
	return -EPERM;
}

static inline int coul_drv_low_temp_opt(void)
{
	return 0;
}

static inline int coul_drv_battery_cc(void)
{
	return -EPERM;
}

static inline int coul_drv_battery_fifo_depth(void)
{
	return 0;
}

static inline int coul_drv_battery_ufcapacity_tenth(void)
{
	return 0;
}

static inline int coul_drv_battery_fifo_curr(unsigned int index)
{
	return 0;
}

static inline int coul_drv_convert_regval2ua(unsigned int reg_val)
{
	return -EPERM;
}

static inline int coul_drv_convert_regval2uv(unsigned int reg_val)
{
	return -EPERM;
}

static inline int coul_drv_convert_regval2temp(unsigned int reg_val)
{
	return -EPERM;
}

static inline int coul_drv_convert_mv2regval(int vol_mv)
{
	return -EPERM;
}

static inline int coul_drv_cal_uah_by_ocv(int ocv_uv, int *ocv_soc_uah)
{
	return -EPERM;
}

static inline int coul_drv_convert_temp_to_adc(int temp)
{
	return -EPERM;
}

static inline int coul_drv_coul_chip_temperature(void)
{
	return -99; /* -99: INVALID_TEMP */
}

static inline int coul_drv_battery_cc_uah(void)
{
	return -EPERM;
}

static inline int coul_drv_battery_removed_before_boot(void)
{
	return -1;
}

static inline int coul_drv_battery_get_qmax(void)
{
	return 0;
}

static inline int coul_drv_battery_temperature_raw(int *raw_temp)
{
	return -EPERM;
}

static inline int coul_drv_battery_temperature_permille(int *raw_temp)
{
	return -EPERM;
}

static inline int coul_drv_get_ocv_by_soc(int temp, int soc)
{
	return -EPERM;
}

static inline int coul_drv_get_soc_by_ocv(int temp, int ocv)
{
	return -EPERM;
}

static inline int coul_drv_get_ocv(void)
{
	return -EPERM;
}

static inline int coul_drv_get_polar_avg(void)
{
	return 0;
}

static inline int coul_drv_get_polar_peak(void)
{
	return 0;
}

static inline int coul_drv_battery_update_basp_policy(
	unsigned int level, unsigned int nondc_volt_dec)
{
	return -EPERM;
}

static inline int coul_drv_battery_get_recrod_fcc(unsigned int size,
	unsigned int *records)
{
	return -EPERM;
}

static inline int coul_drv_get_last_powerdown_soc(void)
{
	return -EPERM;
}

static inline int coul_drv_coulobmeter_cur_calibration(int *cur_offset_a, int *cur_offset_b)
{
	return -EPERM;
}

static inline int coul_drv_battery_fifo_vol(unsigned int index)
{
	return 0;
}

static inline int coul_drv_battery_rpcb(void)
{
	return 0;
}

static inline void coul_drv_force_ocv(void)
{
}

static inline int coul_drv_interpolate_pc(int ocv)
{
	return 0;
}

static inline int coul_drv_get_desired_charging_current(void)
{
	return 0;
}

static inline int coul_drv_get_desired_charging_voltage(void)
{
	return -EPERM;
}

static inline int coul_drv_is_smart_battery(void)
{
	return 0;
}

static inline int coul_drv_is_battery_full_charged(void)
{
	return 0;
}
#endif /* CONFIG_COUL_DRV */

#endif /* _COUL_DRV_H */
