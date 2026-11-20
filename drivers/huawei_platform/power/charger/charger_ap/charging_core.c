/*
 * charging_core.c
 *
 * charging_core driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/raid/pq.h>
#ifdef CONFIG_COUL_DRV
#include <linux/power/hisi/coul/coul_drv.h>
#endif
#include <charging_core.h>
#ifdef CONFIG_TCPC_CLASS
#include <huawei_platform/usb/hw_pd_dev.h>
#endif
#include <huawei_platform/power/battery_voltage.h>
#include <chipset_common/hwpower/battery/battery_temp.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <charging_core_debug.h>
#include <huawei_platform/power/battery_temp_fitting.h>

#define HWLOG_TAG charging_core
HWLOG_REGIST();

struct charge_core_info *g_core_info;

static int vdpm_first_run = FIRST_RUN_TRUE;
static int sbatt_running_first = 1;

/* when plugout cable, reset some charging_core paras */
void stop_charging_core_config(void)
{
	hwlog_info("%s\n", __func__);
	vdpm_first_run = FIRST_RUN_TRUE;
}

/*
 * if battery temperature is overheat and battery voltage is high,
 * limit input current to help to discharge, battery voltage from coulometer,
 * temperature from battery NTC temperature, temp_data[] is the table for
 * describe the temperature property.
 */
static void charge_core_overheat_protect(int vbat, int temp,
	struct charge_temp_data temp_data[], struct charge_core_data *data)
{
	static int overheat_count;

	if (!data->high_temp_limit)
		return;

	if (temp >= temp_data[data->temp_level - 1].temp_min) {
		overheat_count++;
		if ((vbat > temp_data[data->temp_level - 1].vterm_temp) &&
			(overheat_count >= OVERHEAT_TIMES)) {
			overheat_count = OVERHEAT_TIMES;
			data->iin = 100; /* min input current */
		}
	} else {
		overheat_count = 0;
	}
	hwlog_info("overheat temp %d vterm %d mV, cur temp %d vbat %d\n",
		temp_data[data->temp_level - 1].temp_min,
		temp_data[data->temp_level - 1].vterm_temp, temp, vbat);
}

/* update the charge parameters in different battery temperature. */
static void charge_core_tbatt_handler(int vbat, int temp,
	struct charge_temp_data temp_data[], struct charge_core_data *data)
{
	int i;
	static int last_i;
	static int last_iin;
	static int last_ichg;
	static int last_vterm;
	static int last_temp;
	static int flag_running_first = 1;

	for (i = 0; i < TEMP_PARA_LEVEL; i++) {
		if ((temp >= temp_data[i].temp_min) &&
			(temp < temp_data[i].temp_max)) {
			if ((last_temp - temp <= 0) ||
				(temp_data[i].temp_max - temp >
				temp_data[i].temp_back) ||
				(abs(last_i - i) > 1) ||
				(flag_running_first == 1)) {
				data->iin = temp_data[i].iin_temp;
				data->ichg = temp_data[i].ichg_temp;
				data->vterm = temp_data[i].vterm_temp;

				/* in high temp, vterm is setted because of jeita */
				if (data->high_temp_limit && (data->vterm ==
					(unsigned int)temp_data[data->temp_level - 1].vterm_temp))
					data->warm_triggered = true;
				else
					data->warm_triggered = false;
			} else {
				data->iin = last_iin;
				data->ichg = last_ichg;
				data->vterm = last_vterm;
			}
			break;
		}
	}
	last_i = i;
	flag_running_first = 0;
	last_temp = temp;
	last_iin = data->iin;
	last_ichg = data->ichg;
	last_vterm = data->vterm;

	hwlog_info("%s: i = %d, temp = %d, data->iin = %u, data->ichg = %u, data->vterm = %u\n",
		__func__, i, temp, data->iin, data->ichg, data->vterm);

	/*
	 * if temperature is overheat and battery voltage is high,
	 * limit input current to help to discharge
	 */
	charge_core_overheat_protect(vbat, temp, temp_data, data);
}

static void charge_core_vbatt_handler(int vbat,
	struct charge_volt_data volt_data[], struct charge_core_data *data)
{
	int i;
	static int last_i;
	static int last_iin;
	static int last_ichg;
	static int last_vbat;
	static int flag_running_first = 1;

	for (i = 0; i < VOLT_PARA_LEVEL; i++) {
		if ((vbat >= volt_data[i].vbat_min) && (vbat < volt_data[i].vbat_max)) {
			if ((last_vbat - vbat <= 0) ||
				(volt_data[i].vbat_max - vbat > volt_data[i].volt_back) ||
				(abs(last_i - i) > 1) ||
				(flag_running_first == 1)) {
				data->iin = data->iin < (unsigned int)(volt_data[i].iin_volt) ?
					data->iin : (unsigned int)(volt_data[i].iin_volt);
				data->ichg = data->ichg < (unsigned int)(volt_data[i].ichg_volt) ?
					data->ichg : (unsigned int)(volt_data[i].ichg_volt);
			} else {
				data->iin = data->iin < (unsigned int)(last_iin) ?
					data->iin : (unsigned int)(last_iin);
				data->ichg = data->ichg < (unsigned int)(last_ichg) ?
					data->ichg : (unsigned int)(last_ichg);
			}
			break;
		}
	}
	last_i = i;
	flag_running_first = 0;
	last_vbat = vbat;
	last_iin = data->iin;
	last_ichg = data->ichg;
}

static void charge_core_vdpm_by_vol_handler(int vbat,
	struct charge_vdpm_data vdpm_data[], struct charge_core_data *data)
{
	int i;
	static int last_i;
	static int last_vdpm;
	static int last_vbat;

	hwlog_info("vdpm_first_run = %d\n", vdpm_first_run);
	for (i = 0; i < VDPM_PARA_LEVEL; i++) {
		if ((vbat >= vdpm_data[i].cap_min) && (vbat < vdpm_data[i].cap_max)) {
			if (((last_vbat - vbat <= 0) && (last_i <= i)) ||
				(vdpm_data[i].cap_max - vbat > vdpm_data[i].cap_back) ||
				(abs(last_i - i) > 1) ||
				(vdpm_first_run == FIRST_RUN_TRUE)) {
				data->vdpm = vdpm_data[i].vin_dpm;
				last_i = i;
			} else {
				data->vdpm = last_vdpm;
			}
			break;
		}
	}

	vdpm_first_run = FIRST_RUN_FALSE;
	last_vbat = vbat;
	last_vdpm = data->vdpm;

	hwlog_info("battery voltage is %d mv, vindpm set to %u mV\n",
		vbat, data->vdpm);
}

static void charge_core_vdpm_by_cap_handler(int cbat,
	struct charge_vdpm_data vdpm_data[], struct charge_core_data *data)
{
	int i;
	static int last_i;
	static int last_vdpm;
	static int last_cbat;
	static int flag_running_first = 1;

	for (i = 0; i < VDPM_PARA_LEVEL; i++) {
		if ((cbat >= vdpm_data[i].cap_min) && (cbat < vdpm_data[i].cap_max)) {
			if ((last_cbat - cbat <= 0) ||
				(vdpm_data[i].cap_max - cbat > vdpm_data[i].cap_back) ||
				(abs(last_i - i) > 1) ||
				(flag_running_first == 1)) {
				data->vdpm = vdpm_data[i].vin_dpm;
			} else {
				data->vdpm = last_vdpm;
			}
			break;
		}
	}

	last_i = i;
	flag_running_first = 0;
	last_cbat = cbat;
	last_vdpm = data->vdpm;

	hwlog_debug("battery capacity is %%%d, vindpm set to %u mV\n", cbat,
		data->vdpm);
}

static void charge_core_vdpm_handler(int cbatt,
	int vbatt, struct charge_vdpm_data vdpm_data[],
	struct charge_core_data *data)
{
	switch (data->vdpm_control_type) {
	case VDPM_BY_CAPACITY:
		hwlog_debug("vdpm_type is set by battery capacity\n");
		charge_core_vdpm_by_cap_handler(cbatt, vdpm_data, data);
		break;
	case VDPM_BY_VOLTAGE:
		hwlog_debug("vdpm_type is set by battery voltage\n");
		charge_core_vdpm_by_vol_handler(vbatt, vdpm_data, data);
		break;
	default:
		hwlog_err("vdpm_type is set wrong\n");
		break;
	}
}

static int segment_flag;
static void charge_core_sbatt_by_vbat_ichg_handler(int vbat,
	struct charge_segment_data segment_data[],
	struct charge_core_data *data)
{
	int i, ichg;
	static int last_i;
	static int last_vterm;
	static int last_ichg;
	static int last_vbat;

	ichg = -coul_drv_battery_current();

	/* choose index of segment_data */
	for (i = 0; i < (int)(data->segment_level); i++) {
		if ((vbat >= segment_data[i].vbat_min) &&
			(vbat < segment_data[i].vbat_max) &&
			(ichg < segment_data[i].ichg_segment)) {
			if ((last_i - i <= 0) ||
				(segment_data[i].vbat_max - vbat > segment_data[i].volt_back) ||
				(abs(last_i - i) > 1) ||
				(sbatt_running_first == 1)) {
				/* do nothing, just get index "i" */
			} else {
				i = i + 1;
			}
			break;
		}
	}

	/* if cannot choose right index, keep last index */
	if (i == data->segment_level)
		i = last_i;

	/* set ichg and vterm according to index  only when two consecutive index is the same ,or keep last ichg and vterm */
	if ((last_i == i) || (sbatt_running_first == 1)) {
		data->ichg = data->ichg < (unsigned int)(segment_data[i].ichg_segment) ?
			data->ichg : (unsigned int)(segment_data[i].ichg_segment);
		data->vterm = data->vterm < (unsigned int)(segment_data[i].vterm_segment) ?
			data->vterm : (unsigned int)(segment_data[i].vterm_segment);
	} else {
		data->ichg = data->ichg < (unsigned int)(last_ichg) ?
			data->ichg : (unsigned int)(last_ichg);
		data->vterm = data->vterm < (unsigned int)(last_vterm) ?
			data->vterm : (unsigned int)(last_vterm);
	}
	hwlog_info("%s: ichg = %d, vbat = %d, last_i = %d, i = %d, last_ichg = %u, segment_data[i].ichg_segment = %d, data->ichg = %u, last_vterm = %u, segment_data[i].vterm_segment = %d, data->vterm = %u\n",
		__func__, ichg, vbat, last_i, i, last_ichg, segment_data[i].ichg_segment,
		data->ichg, last_vterm, segment_data[i].vterm_segment, data->vterm);

	last_i = i;
	segment_flag = i;
	sbatt_running_first = 0;
	last_vbat = vbat;
	last_ichg = data->ichg;
	last_vterm = data->vterm;
}

static void charge_core_sbatt_by_vbat_handler(int vbat,
	struct charge_segment_data segment_data[],
	struct charge_core_data *data)
{
	int i;

	for (i = 0; i < (int)(data->segment_level); i++) {
		if ((vbat >= segment_data[i].vbat_min) &&
			(vbat < segment_data[i].vbat_max))
			break;
	}
	if (i == data->segment_level)
		i -= 1;

	segment_flag = i;

	data->ichg = data->ichg < (unsigned int)(segment_data[i].ichg_segment) ?
		data->ichg : (unsigned int)(segment_data[i].ichg_segment);
	data->vterm = data->vterm < (unsigned int)(segment_data[i].vterm_segment) ?
		data->vterm : (unsigned int)(segment_data[i].vterm_segment);
}

static void charge_core_sbatt_handler(int vbat,
	struct charge_segment_data segment_data[],
	struct charge_core_data *data)
{
	/* not fcp do not use segment charging strategy */
	if ((fcp_get_stage_status() != FCP_STAGE_SUCESS)
#ifdef CONFIG_TCPC_CLASS
		&& (pd_dpm_get_high_power_charging_status() != true)
#endif
	) {
		sbatt_running_first = 1;
		return;
	}

	switch (data->segment_type) {
	case SEGMENT_TYPE_BY_VBAT_ICHG:
		hwlog_debug("case = SEGMENT_TYPE_BY_VBAT_ICHG\n");
		charge_core_sbatt_by_vbat_ichg_handler(vbat, segment_data, data);
		break;
	case SEGMENT_TYPE_BY_VBAT:
		hwlog_debug("case = SEGMENT_TYPE_BY_VBAT\n");
		charge_core_sbatt_by_vbat_handler(vbat, segment_data, data);
		break;
	case SEGMENT_TYPE_BY_DEFAULT:
		hwlog_debug("case = SEGMENT_TYPE_BY_DEFAULT\n");
		break;
	default:
		break;
	}
}

static void charge_core_protect_inductance_handler(int cbat,
	struct charge_inductance_data inductance_data[],
	struct charge_core_data *data)
{
	int i;
	static int last_i;
	static int last_iin;
	static int last_cbat;
	static int flag_running_first = 1;

	/* not fcp do not use segment charging strategy */
	if ((fcp_get_stage_status() != FCP_STAGE_SUCESS)
#ifdef CONFIG_TCPC_CLASS
		&& (pd_dpm_get_high_power_charging_status() != true)
#endif
		)
		return;
	for (i = 0; i < INDUCTANCE_PARA_LEVEL; i++) {
		if ((cbat >= inductance_data[i].cap_min) && (cbat < inductance_data[i].cap_max)) {
			if ((last_i - i <= 0) ||
				(inductance_data[i].cap_max - cbat > inductance_data[i].cap_back) ||
				(abs(last_i - i) > 1) ||
				(flag_running_first == 1)) {
				data->iin = data->iin <
					(unsigned int)(inductance_data[i].iin_inductance) ?
					data->iin :
					(unsigned int)(inductance_data[i].iin_inductance);
			} else {
				data->iin = data->iin < (unsigned int)(last_iin) ?
					data->iin : (unsigned int)(last_iin);
			}
			break;
		}
	}

	last_i = i;
	flag_running_first = 0;
	last_cbat = cbat;
	last_iin = data->iin;

	hwlog_info("%s: battery capacity is %%%d, last_iin set to %u ma, i = %d\n",
		__func__, cbat, data->iin, i);
}

static void charge_core_soh_handler(struct charge_core_data *data)
{
	int fcc = coul_drv_battery_fcc_design();
	int ichg_spec = 0;

	hwlog_info("before, ichg:%u, vterm:%u, segment:%d\n",
		data->ichg, data->vterm, segment_flag);

	if (data->vterm_bsoh > 0)
		data->vterm = data->vterm <= data->vterm_bsoh ?
			data->vterm : data->vterm_bsoh;

	if (data->ichg_bsoh > 0)
		data->ichg = data->ichg <= data->ichg_bsoh ?
			data->ichg : data->ichg_bsoh;
	if ((segment_flag > 0) && (fcc > 0)) {
		ichg_spec = fcc * MAX_BATT_CHARGE_CUR_RATIO /
			BASP_PARA_SCALE;
		data->ichg = data->ichg <= ichg_spec ?
			data->ichg : ichg_spec;
	}

	hwlog_info("after, ichg:%u, vterm:%u,"
		"ichg_bsoh:%u, ichg_spec:%d, vterm_bsoh:%u\n",
		data->ichg, data->vterm, data->ichg_bsoh, ichg_spec,
		data->vterm_bsoh);
}

struct charge_core_data *charge_core_get_params(void)
{
	struct charge_core_info *di = g_core_info;
	struct charge_temp_data *temp_para = NULL;
	int tbatt = 0;
	int vbatt = hw_battery_voltage(BAT_ID_MAX);
	int cbatt = coul_drv_battery_capacity();

	if (!di) {
		hwlog_err("%s, charge_core_info is NULL\n", __func__);
		return NULL;
	}

	bat_temp_get_rt_temperature(BAT_TEMP_MIXED, &tbatt);

	temp_para = (struct charge_temp_data *)btf_charge_temp_data_eqr();
	if (!temp_para)
		temp_para = di->temp_para;
	charge_core_tbatt_handler(vbatt, tbatt, temp_para, &di->data);
	charge_core_vbatt_handler(vbatt, di->volt_para, &di->data);
	charge_core_vdpm_handler(cbatt, vbatt, di->vdpm_para, &di->data);
	charge_core_sbatt_handler(vbatt, di->segment_para, &di->data);
	charge_core_protect_inductance_handler(cbatt, di->inductance_para, &di->data);
	charge_core_soh_handler(&di->data);

	return &di->data;
}

static int charge_core_battery_data(struct charge_core_info *di)
{
	int i;
	struct chrg_para_lut *p_batt_data = NULL;

	p_batt_data = coul_drv_battery_charge_params();
	if (!p_batt_data) {
		hwlog_err("get battery params fail\n");
		return -EINVAL;
	}

	di->data.temp_level = (unsigned int)((p_batt_data->temp_len) / TEMP_PARA_TOTAL);
	for (i = 0; i < (p_batt_data->temp_len) / TEMP_PARA_TOTAL; i++) {
		di->temp_para[i].temp_min =
			p_batt_data->temp_data[i][TEMP_PARA_TEMP_MIN];
		di->temp_para[i].temp_max =
			p_batt_data->temp_data[i][TEMP_PARA_TEMP_MAX];
		di->temp_para[i].iin_temp =
			p_batt_data->temp_data[i][TEMP_PARA_IIN];
		di->temp_para[i].ichg_temp =
			p_batt_data->temp_data[i][TEMP_PARA_ICHG];
		di->temp_para[i].vterm_temp =
			p_batt_data->temp_data[i][TEMP_PARA_VTERM];
		di->temp_para[i].temp_back =
			p_batt_data->temp_data[i][TEMP_PARA_TEMP_BACK];

		if (di->temp_para[i].iin_temp == -1)
			di->temp_para[i].iin_temp = di->data.iin_max;
		if (di->temp_para[i].ichg_temp == -1)
			di->temp_para[i].ichg_temp = di->data.ichg_max;

		hwlog_debug("temp_min = %d,temp_max = %d,iin_temp = %d,ichg_temp = %d,vterm_temp = %d,temp_back = %d\n",
			di->temp_para[i].temp_min, di->temp_para[i].temp_max,
			di->temp_para[i].iin_temp, di->temp_para[i].ichg_temp,
			di->temp_para[i].vterm_temp, di->temp_para[i].temp_back);

		if ((di->temp_para[i].temp_min < BATTERY_TEMPERATURE_MIN) ||
			(di->temp_para[i].temp_min > BATTERY_TEMPERATURE_MAX) ||
			(di->temp_para[i].temp_max < BATTERY_TEMPERATURE_MIN) ||
			(di->temp_para[i].temp_max > BATTERY_TEMPERATURE_MAX) ||
			(di->temp_para[i].iin_temp < CHARGE_CURRENT_0000_MA) ||
			(di->temp_para[i].iin_temp > CHARGE_CURRENT_4000_MA) ||
			(di->temp_para[i].ichg_temp < CHARGE_CURRENT_0000_MA) ||
			(di->temp_para[i].ichg_temp > CHARGE_CURRENT_4000_MA) ||
			(di->temp_para[i].vterm_temp < di->data.vterm_low_th) ||
			(di->temp_para[i].vterm_temp > di->data.vterm_high_th) ||
			(di->temp_para[i].temp_back < BATTERY_TEMPERATURE_0_C) ||
			(di->temp_para[i].temp_back > BATTERY_TEMPERATURE_5_C)) {
			hwlog_err("the temp_para value is out of range\n");
			return -EINVAL;
		}
	}
	for (i = 0; i < (p_batt_data->volt_len) / VOLT_PARA_TOTAL; i++) {
		di->volt_para[i].vbat_min =
			p_batt_data->volt_data[i][VOLT_PARA_VOLT_MIN];
		di->volt_para[i].vbat_max =
			p_batt_data->volt_data[i][VOLT_PARA_VOLT_MAX];
		di->volt_para[i].iin_volt =
			p_batt_data->volt_data[i][VOLT_PARA_IIN];
		di->volt_para[i].ichg_volt =
			p_batt_data->volt_data[i][VOLT_PARA_ICHG];
		di->volt_para[i].volt_back =
			p_batt_data->volt_data[i][VOLT_PARA_VOLT_BACK];

		if (di->volt_para[i].iin_volt == -1)
			di->volt_para[i].iin_volt = di->data.iin_max;
		if (di->volt_para[i].ichg_volt == -1)
			di->volt_para[i].ichg_volt = di->data.ichg_max;

		hwlog_debug("vbat_min = %d,vbat_max = %d,iin_volt = %d,ichg_volt = %d,volt_back = %d\n",
			di->volt_para[i].vbat_min, di->volt_para[i].vbat_max,
			di->volt_para[i].iin_volt, di->volt_para[i].ichg_volt,
			di->volt_para[i].volt_back);

		if ((di->volt_para[i].vbat_min < BATTERY_VOLTAGE_MIN_MV) ||
			(di->volt_para[i].vbat_min > BATTERY_VOLTAGE_MAX_MV) ||
			(di->volt_para[i].vbat_max < BATTERY_VOLTAGE_MIN_MV) ||
			(di->volt_para[i].vbat_max > BATTERY_VOLTAGE_MAX_MV) ||
			(di->volt_para[i].iin_volt < CHARGE_CURRENT_0000_MA) ||
			(di->volt_para[i].iin_volt > CHARGE_CURRENT_4000_MA) ||
			(di->volt_para[i].ichg_volt < CHARGE_CURRENT_0000_MA) ||
			(di->volt_para[i].ichg_volt > CHARGE_CURRENT_4000_MA) ||
			(di->volt_para[i].volt_back < BATTERY_VOLTAGE_0000_MV) ||
			(di->volt_para[i].volt_back > BATTERY_VOLTAGE_0200_MV)) {
			hwlog_err("the volt_para value is out of range\n");
			return -EINVAL;
		}
	}

	/* Initialize segment charging param */
	di->data.segment_level =
		(p_batt_data->segment_len) / SEGMENT_PARA_TOTAL;
	for (i = 0; i < (int)(di->data.segment_level); i++) {
		di->segment_para[i].vbat_min =
			p_batt_data->segment_data[i][SEGMENT_PARA_VOLT_MIN];
		di->segment_para[i].vbat_max =
			p_batt_data->segment_data[i][SEGMENT_PARA_VOLT_MAX];
		di->segment_para[i].ichg_segment =
			p_batt_data->segment_data[i][SEGMENT_PARA_ICHG];
		di->segment_para[i].vterm_segment =
			p_batt_data->segment_data[i][SEGMENT_PARA_VTERM];
		di->segment_para[i].volt_back =
			p_batt_data->segment_data[i][SEGMENT_PARA_VOLT_BACK];

		hwlog_info("segment param: vbat_min = %d,vbat_max = %d,ichg_segment = %d,vterm_segment = %d,volt_back = %d\n",
			di->segment_para[i].vbat_min, di->segment_para[i].vbat_max,
			di->segment_para[i].ichg_segment,
			di->segment_para[i].vterm_segment,
			di->segment_para[i].volt_back);

		if ((di->segment_para[i].vbat_min < BATTERY_VOLTAGE_MIN_MV) ||
			(di->segment_para[i].vbat_min > BATTERY_VOLTAGE_MAX_MV) ||
			(di->segment_para[i].vbat_max < BATTERY_VOLTAGE_MIN_MV) ||
			(di->segment_para[i].vbat_max > BATTERY_VOLTAGE_MAX_MV) ||
			(di->segment_para[i].ichg_segment < CHARGE_CURRENT_0000_MA) ||
			(di->segment_para[i].ichg_segment > CHARGE_CURRENT_MAX_MA) ||
			(di->segment_para[i].vterm_segment < di->data.vterm_low_th) ||
			(di->segment_para[i].vterm_segment > di->data.vterm_high_th) ||
			(di->segment_para[i].volt_back < BATTERY_VOLTAGE_0000_MV) ||
			(di->segment_para[i].volt_back > BATTERY_VOLTAGE_0200_MV)) {
			hwlog_err("the segment_para value is out of range\n");
			return -EINVAL;
		}
	}

	return 0;
}

#ifdef CONFIG_WIRELESS_CHARGER
static int charge_core_parse_wireless_charge_para(struct device_node *np,
	struct charge_core_info *di)
{
	int ret = of_property_read_u32(np, "iin_wireless", &(di->data.iin_wireless));

	if (ret) {
		hwlog_err("get iin_wireless failed\n");
		return -EINVAL;
	}
	hwlog_info("iin_wireless = %u\n", di->data.iin_wireless);
	ret = of_property_read_u32(np, "ichg_wireless", &(di->data.ichg_wireless));
	if (ret) {
		hwlog_err("get ichg_wireless failed\n");
		return -EINVAL;
	}
	hwlog_info("ichg_wireless = %u\n", di->data.ichg_wireless);
	return ret;
}
#endif

static int charge_core_parse_vdpm_para(struct device_node *np, struct charge_core_info *di)
{
	int i;
	int array_len;
	int idata;
	const char *chrg_data_string = NULL;
	unsigned int vdpm_control_type = VDPM_BY_CAPACITY;
	unsigned int vdpm_buf_limit = VDPM_DELTA_LIMIT_5;
	/* vdpm control type : 0 vdpm controlled by cbat, 1 vdpm controlled by vbat */
	int ret = of_property_read_u32(np, "vdpm_control_type", &(vdpm_control_type));

	if (ret)
		hwlog_err("get vdpm_control_type failed, use default config\n");
	di->data.vdpm_control_type = vdpm_control_type;
	hwlog_info("vdpm_control_type = %u\n", di->data.vdpm_control_type);
	/* vdpm buffer setting */
	ret = of_property_read_u32(np, "vdpm_buf_limit", &(vdpm_buf_limit));
	if (ret)
		hwlog_err("get vdpm_buf_limit failed,use default config\n");
	di->data.vdpm_buf_limit = vdpm_buf_limit;
	hwlog_info("vdpm_buf_limit = %u\n", di->data.vdpm_buf_limit);
	array_len = of_property_count_strings(np, "vdpm_para");
	if ((array_len <= 0) || (array_len % VDPM_PARA_TOTAL != 0)) {
		hwlog_err("vdpm_para is invaild,please check vdpm_para number\n");
		return -EINVAL;
	}

	if (array_len > VDPM_PARA_LEVEL * VDPM_PARA_TOTAL) {
		array_len = VDPM_PARA_LEVEL * VDPM_PARA_TOTAL;
		hwlog_err("vdpm_para is too long,use only front %d paras\n",
			array_len);
		return -EINVAL;
	}
	/* data reset to 0 */
	memset(di->vdpm_para, 0, VDPM_PARA_LEVEL * sizeof(struct charge_vdpm_data));

	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np, "vdpm_para", i,
			&chrg_data_string);
		if (ret) {
			hwlog_err("get vdpm_para failed\n");
			return -EINVAL;
		}

		idata = simple_strtol(chrg_data_string, NULL, 10);
		switch (i % VDPM_PARA_TOTAL) {
		case VDPM_PARA_CAP_MIN:
			if ((idata < VDPM_CBAT_MIN) || (idata > VDPM_CBAT_MAX)) {
				hwlog_err("the vdpm_para cap_min is out of range\n");
				return -EINVAL;
			}
			di->vdpm_para[i / (VDPM_PARA_TOTAL)].cap_min = idata;
			break;
		case VDPM_PARA_CAP_MAX:
			if ((idata < VDPM_CBAT_MIN) || (idata > VDPM_CBAT_MAX)) {
				hwlog_err("the vdpm_para cap_max is out of range\n");
				return -EINVAL;
			}
			di->vdpm_para[i / (VDPM_PARA_TOTAL)].cap_max = idata;
			break;
		case VDPM_PARA_DPM:
			if ((idata < VDPM_VOLT_MIN) || (idata > VDPM_VOLT_MAX)) {
				hwlog_err("the vdpm_para vin_dpm is out of range\n");
				return -EINVAL;
			}
			di->vdpm_para[i / (VDPM_PARA_TOTAL)].vin_dpm = idata;
			break;
		case VDPM_PARA_CAP_BACK:
			if ((idata < 0) || (idata > (int)(di->data.vdpm_buf_limit))) {
				hwlog_err("the vdpm_para cap_back is out of range\n");
				return -EINVAL;
			}
			di->vdpm_para[i / (VDPM_PARA_TOTAL)].cap_back = idata;
			break;
		default:
			hwlog_err("get vdpm_para failed\n");
		}
		hwlog_info("di->vdpm_para[%d][%d] = %d\n",
			i / (VDPM_PARA_TOTAL), i % (VDPM_PARA_TOTAL), idata);
	}

	return ret;
}

static void charge_core_parse_high_temp_limit(struct device_node *np,
	struct charge_core_info *di)
{
	unsigned int high_temp_limit = 0;
	int ret = of_property_read_u32(np, "high_temp_limit", &high_temp_limit);

	if (ret)
		hwlog_err("get high_temp_limit failed,use default config\n");
	di->data.high_temp_limit = high_temp_limit;

	hwlog_info("high_temp_limit = %u\n", di->data.high_temp_limit);
}

static int charging_core_parse_iterm(struct charge_core_info *di, struct device_node *np)
{
	int ret, i, array_len;
	int iterm = 0;
	const char *outstring = NULL;
	char *batt_brand = coul_drv_battery_brand();
	char buff[BATT_BRAND_STRING_MAX] = {0};

	ret = of_property_read_u32(np, "iterm", &(di->data.iterm));
	if (ret) {
		hwlog_err("get iterm failed\n");
		return -EINVAL;
	}
	hwlog_info("iterm = %u\n", di->data.iterm);

	/*
	 * check if iterm para is set
	 * iterm is different, as the model of the battery varies
	 */
	array_len = of_property_count_strings(np, "iterm_para");
	if ((array_len <= 0) || (array_len % CHARGE_ITERM_PARA_TOTAL != 0)) {
		hwlog_err("iterm_para is invaild, not necessay, please check\n");
		return 0;
	}
	if (array_len > BATT_BRAND_NUM_MAX * CHARGE_ITERM_PARA_TOTAL) {
		hwlog_err("iterm_para is too long, please check\n");
		return 0;
	}
	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np, "iterm_para", i, &outstring);
		if (ret) {
			hwlog_err("get iterm_para failed\n");
			return 0;
		}
		switch (i % CHARGE_ITERM_PARA_TOTAL) {
		case CHARGE_ITERM_PARA_BATT_BRAND:
			if (strlen(outstring) < BATT_BRAND_STRING_MAX)
				strncpy(buff, outstring, strlen(outstring));
			break;
		case CHARGE_ITERM_PARA_ITERM:
			iterm = simple_strtol(outstring, NULL, 10);
			break;
		default:
			hwlog_err("iterm_para get failed\n");
			break;
		}
		/* match battery model */
		if ((i % CHARGE_ITERM_PARA_TOTAL) == CHARGE_ITERM_PARA_ITERM) {
			if (!strcmp(buff, batt_brand)) {
				di->data.iterm = iterm;
				hwlog_info("update iterm = %u\n", di->data.iterm);
				break;
			}
			memset(buff, 0, sizeof(buff));
		}
	}
	return 0;
}

static int charge_core_parse_dts(struct device_node *np,
	struct charge_core_info *di)
{
	int i;
	int array_len;
	int idata;
	const char *chrg_data_string = NULL;
	/* ac charge current */
	int ret = of_property_read_u32(np, "iin_ac", &(di->data.iin_ac));

	if (ret) {
		hwlog_err("get iin_ac failed\n");
		return -EINVAL;
	}
	hwlog_debug("iin_ac = %u\n", di->data.iin_ac);
	ret = of_property_read_u32(np, "ichg_ac", &(di->data.ichg_ac));
	if (ret) {
		hwlog_err("get ichg_ac failed\n");
		return -EINVAL;
	}
	hwlog_debug("ichg_ac = %u\n", di->data.ichg_ac);

	/* battery cell num */
	power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "battery_cell_num",
		&(di->data.battery_cell_num), 1);
	di->data.vterm_low_th = di->data.battery_cell_num * BATTERY_VOLTAGE_3200_MV;
	di->data.vterm_high_th = di->data.battery_cell_num * BATTERY_VOLTAGE_4500_MV;
	hwlog_debug("battery cell num = %u\n", di->data.battery_cell_num);

	/* fcp charge current */
	ret = of_property_read_u32(np, "iin_fcp", &(di->data.iin_fcp));
	if (ret) {
		hwlog_info("get iin_fcp failed, use iin_ac's value instead\n");
		di->data.iin_fcp = di->data.iin_ac;
	}
	hwlog_debug("iin_fcp = %u\n", di->data.iin_fcp);
	ret = of_property_read_u32(np, "ichg_fcp", &(di->data.ichg_fcp));
	if (ret) {
		hwlog_info("get ichg_fcp failed, use ichg_ac's value instead\n");
		di->data.ichg_fcp = di->data.ichg_ac;
	}
	hwlog_debug("ichg_fcp = %u\n", di->data.ichg_fcp);

	/* usb charge current */
	ret = of_property_read_u32(np, "iin_usb", &(di->data.iin_usb));
	if (ret) {
		hwlog_err("get iin_usb failed\n");
		return -EINVAL;
	}
	hwlog_debug("iin_usb = %u\n", di->data.iin_usb);
	ret = of_property_read_u32(np, "ichg_usb", &(di->data.ichg_usb));
	if (ret) {
		hwlog_err("get ichg_usb failed\n");
		return -EINVAL;
	}
	hwlog_debug("ichg_usb = %u\n", di->data.ichg_usb);
	/* nonstandard charge current */
	ret = of_property_read_u32(np, "iin_nonstd", &(di->data.iin_nonstd));
	if (ret) {
		hwlog_err("get iin_nonstd failed\n");
		return -EINVAL;
	}
	hwlog_debug("iin_nonstd = %u\n", di->data.iin_nonstd);
	ret = of_property_read_u32(np, "ichg_nonstd", &(di->data.ichg_nonstd));
	if (ret) {
		hwlog_err("get ichg_nonstd failed\n");
		return -EINVAL;
	}
	hwlog_debug("ichg_nonstd = %u\n", di->data.ichg_nonstd);

	/* normal charge current for abnormal scp adaptor */
	if (of_property_read_u32(np, "iin_nor_scp", &di->data.iin_nor_scp))
		di->data.iin_nor_scp = di->data.iin_ac;
	hwlog_debug("iin_nor_scp = %u\n", di->data.iin_nor_scp);
	if (of_property_read_u32(np, "ichg_nor_scp", &di->data.ichg_nor_scp))
		di->data.ichg_nor_scp = di->data.ichg_ac;
	hwlog_debug("ichg_nor_scp = %u\n", di->data.ichg_nor_scp);

	/* Charging Downstream Port */
	ret = of_property_read_u32(np, "iin_bc_usb", &(di->data.iin_bc_usb));
	if (ret) {
		hwlog_err("get iin_bc_usb failed\n");
		return -EINVAL;
	}
	hwlog_debug("iin_bc_usb = %u\n", di->data.iin_bc_usb);
	ret = of_property_read_u32(np, "ichg_bc_usb", &(di->data.ichg_bc_usb));
	if (ret) {
		hwlog_err("get ichg_bc_usb failed\n");
		return -EINVAL;
	}
	hwlog_debug("ichg_bc_usb = %u\n", di->data.ichg_bc_usb);
	/* VR Charge current */
	ret = of_property_read_u32(np, "iin_vr", &(di->data.iin_vr));
	if (ret) {
		hwlog_err("get iin_vr failed\n");
		return -EINVAL;
	}
	hwlog_debug("iin_vr = %u\n", di->data.iin_vr);
	ret = of_property_read_u32(np, "ichg_vr", &(di->data.ichg_vr));
	if (ret) {
		hwlog_err("get ichg_vr failed\n");
		return -EINVAL;
	}
	hwlog_debug("ichg_vr = %u\n", di->data.ichg_vr);
	/* weaksource charge current */
	ret = of_property_read_u32(np, "iin_weaksource", &(di->data.iin_weaksource));
	if (ret) {
		hwlog_info("get iin_weaksource failed, sign with an invalid number\n");
		di->data.iin_weaksource = INVALID_CURRENT_SET;
	}
	hwlog_debug("iin_weaksource = %u\n", di->data.iin_weaksource);
	/* terminal current */
	ret = charging_core_parse_iterm(di, np);
	if (ret) {
		hwlog_err("get iterm failed\n");
		return -EINVAL;
	}
	/* otg current */
	ret = of_property_read_u32(np, "otg_curr", &(di->data.otg_curr));
	if (ret) {
		hwlog_err("get otg_curr failed\n");
		return -EINVAL;
	}
	hwlog_debug("otg_curr = %u\n", di->data.otg_curr);
	/* segment para type */
	ret = of_property_read_u32(np, "segment_type", &(di->data.segment_type));
	if (ret) {
		hwlog_err("get segment_type failed\n");
		return -EINVAL;
	}

	/* TypeC High mode current */
	ret = of_property_read_u32(np, "typec_support",
		&(di->data.typec_support));
	if (ret) {
		hwlog_err("get typec support flag\n");
		return -EINVAL;
	}
	hwlog_info("typec support flag = %u\n", di->data.typec_support);

	ret = of_property_read_u32(np, "iin_typech", &(di->data.iin_typech));
	if (ret) {
		hwlog_err("get typec high mode ibus curr failed\n");
		return -EINVAL;
	}
	hwlog_info("typec high mode ibus curr = %u\n", di->data.iin_typech);
	ret = of_property_read_u32(np, "ichg_typech", &(di->data.ichg_typech));
	if (ret) {
		hwlog_err("get typec high mode ibat curr failed\n");
		return -EINVAL;
	}
	hwlog_info("typec high mode ibat curr = %u\n", di->data.ichg_typech);

	charge_core_parse_high_temp_limit(np, di);
#ifdef CONFIG_WIRELESS_CHARGER
	ret = charge_core_parse_wireless_charge_para(np, di);
	if (ret)
		hwlog_err("get wireless charge para failed\n");
#endif
	ret = charge_core_parse_vdpm_para(np, di);
	if (ret) {
		hwlog_err("get vdpm_para para failed\n");
		return -EINVAL;
	}

	/* inductance_para, data reset to 0 */
	memset(di->inductance_para, 0, INDUCTANCE_PARA_LEVEL *
		sizeof(struct charge_inductance_data));

	array_len = of_property_count_strings(np, "inductance_para");
	if ((array_len <= 0) || (array_len % INDUCTANCE_PARA_TOTAL != 0)) {
		hwlog_err("inductance_para is invaild,please check inductance_para number\n");
		return -EINVAL;
	}
	if (array_len > INDUCTANCE_PARA_LEVEL * INDUCTANCE_PARA_TOTAL) {
		array_len = INDUCTANCE_PARA_LEVEL * INDUCTANCE_PARA_TOTAL;
		hwlog_err("inductance_para is too long,use only front %d paras\n",
			array_len);
		return -EINVAL;
	}

	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np, "inductance_para", i, &chrg_data_string);
		if (ret) {
			hwlog_err("get inductance_para failed\n");
			return -EINVAL;
		}

		idata = simple_strtol(chrg_data_string, NULL, 10);
		switch (i % INDUCTANCE_PARA_TOTAL) {
		case INDUCTANCE_PARA_CAP_MIN:
			if ((idata < INDUCTANCE_CBAT_MIN) || (idata > INDUCTANCE_CBAT_MAX)) {
				hwlog_err("the inductance_para cap_min is out of range\n");
				return -EINVAL;
			}
			di->inductance_para[i / (INDUCTANCE_PARA_TOTAL)].cap_min = idata;
			break;
		case INDUCTANCE_PARA_CAP_MAX:
			if ((idata < INDUCTANCE_CBAT_MIN) || (idata > INDUCTANCE_CBAT_MAX)) {
				hwlog_err("the inductance_para cap_max is out of range\n");
				return -EINVAL;
			}
			di->inductance_para[i / (INDUCTANCE_PARA_TOTAL)].cap_max = idata;
			break;
		case INDUCTANCE_PARA_IIN:
			if ((idata < INDUCTANCE_IIN_MIN) || (idata > INDUCTANCE_IIN_MAX)) {
				hwlog_err("the inductance_para iin is out of range\n");
				return -EINVAL;
			}
			di->inductance_para[i / (INDUCTANCE_PARA_TOTAL)].iin_inductance = idata;
			break;
		case INDUCTANCE_PARA_CAP_BACK:
			if ((idata < 0) || (idata > INDUCTANCE_CAP_DETA)) {
				hwlog_err("the inductance_para cap_back is out of range\n");
				return -EINVAL;
			}
			di->inductance_para[i / (INDUCTANCE_PARA_TOTAL)].cap_back = idata;
			break;
		default:
			hwlog_err("get vdpm_para failed\n");
		}
		hwlog_info("di->inductance_para[%d][%d] = %d\n",
			i / (INDUCTANCE_PARA_TOTAL),
			i % (INDUCTANCE_PARA_TOTAL), idata);
	}

	if (power_cmdline_is_factory_mode() && !coul_drv_is_battery_exist()) {
		di->data.iin_ac = CHARGE_CURRENT_2000_MA;
		di->data.ichg_ac = CHARGE_CURRENT_1900_MA;
		di->data.iin_bc_usb = CHARGE_CURRENT_2000_MA;
		di->data.ichg_bc_usb = CHARGE_CURRENT_1900_MA;
		hwlog_info("factory version,iin_ac = %u mA,ichg_ac %u mA,iin_bc_usb = %u mA,ichg_bc_usb = %u mA\n",
			di->data.iin_ac, di->data.ichg_ac, di->data.iin_bc_usb,
			di->data.ichg_bc_usb);
	}
	di->data.iin_max = di->data.iin_ac < di->data.iin_fcp ? di->data.iin_fcp : di->data.iin_ac;
	di->data.ichg_max = di->data.ichg_ac < di->data.ichg_fcp ?
		di->data.ichg_fcp : di->data.ichg_ac;
#ifdef CONFIG_WIRELESS_CHARGER
	if (di->data.iin_wireless > di->data.iin_max)
		di->data.iin_max = di->data.iin_wireless;
	if (di->data.ichg_wireless > di->data.ichg_max)
		di->data.ichg_max = di->data.ichg_wireless;
#endif
	di->data.iin_max = di->data.iin_nor_scp > di->data.iin_max ?
		di->data.iin_nor_scp : di->data.iin_max;
	di->data.ichg_max = di->data.ichg_nor_scp > di->data.ichg_max ?
		di->data.ichg_nor_scp : di->data.ichg_max;
	hwlog_info("iin_max = %u mA,ichg_max %u mA\n", di->data.iin_max, di->data.ichg_max);
	return 0;
}

static int charge_core_probe(struct platform_device *pdev)
{
	struct charge_core_info *di = NULL;
	int ret;
	struct device_node *np = NULL;

	if (!pdev->dev.of_node) {
		hwlog_err("device_node is NULL\n");
		return -ENOMEM;
	}

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_core_info = di;
	di->dev = &pdev->dev;
	np = di->dev->of_node;
	platform_set_drvdata(pdev, di);

	ret = charge_core_parse_dts(np, di);
	if (ret < 0)
		goto err_batt;

	ret = charge_core_battery_data(di);
	if (ret < 0) {
		hwlog_err("get battery charge data fail\n");
		goto err_batt;
	}

	charging_core_dbg_register(di);
	hwlog_info("charging core probe ok\n");
	return 0;
err_batt:
	platform_set_drvdata(pdev, NULL);
	kfree(di);
	di = NULL;
	g_core_info = NULL;
	return ret;
}

static int charge_core_remove(struct platform_device *pdev)
{
	struct charge_core_info *di = platform_get_drvdata(pdev);

	if (!di) {
		hwlog_err("[%s]di is NULL\n", __func__);
		return -ENODEV;
	}

	platform_set_drvdata(pdev, NULL);
	kfree(di);
	di = NULL;
	g_core_info = NULL;

	return 0;
}

static struct of_device_id charge_core_match_table[] = {
	{
		.compatible = "huawei,charging_core",
		.data = NULL,
	},
	{},
};

static struct platform_driver charge_core_driver = {
	.probe = charge_core_probe,
	.remove = charge_core_remove,
	.driver = {
		.name = "huawei,charging_core",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(charge_core_match_table),
	},
};

int __init charge_core_init(void)
{
	return platform_driver_register(&charge_core_driver);
}

void __exit charge_core_exit(void)
{
	platform_driver_unregister(&charge_core_driver);
}

module_init(charge_core_init);
module_exit(charge_core_exit);
MODULE_AUTHOR("HUAWEI");
MODULE_DESCRIPTION("charging core module driver");
MODULE_LICENSE("GPL");

