/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description:  driver for battery
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

#include <linux/bug.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <securec.h>
#ifdef CONFIG_HUAWEI_DSM
#include <chipset_common/hwpower/common_module/power_dsm.h>
#endif
#ifdef CONFIG_HUAWEI_BATTERY_INFORMATION
#include <huawei_platform/power/batt_info_pub.h>
#endif
#include <linux/power/hisi/battery_data.h>

#define bat_data_debug(fmt, args...) pr_debug("[battery_data]" fmt, ## args)
#define bat_data_info(fmt, args...) pr_info("[battery_data]" fmt, ## args)
#define bat_data_warn(fmt, args...) pr_warn("[battery_data]" fmt, ## args)
#define bat_data_err(fmt, args...) pr_err("[battery_data]" fmt, ## args)

#ifdef BATTERY_DATA_DEBUG
#define bat_info(fmt, args...) bat_data_info(fmt, ## args)
#else
#define bat_info(fmt, args...) do {} while (0)
#endif

/* static variable definition */
static struct coul_battery_data **g_p_data;
/* used to store number of bat types defined in DTS */
static unsigned int g_bat_data_size;
/* used to judege whether bat_drv works fine or not, 1 means yes, 0 means no */
static int g_bat_param_status;
static int g_default_batt_id_index = -1;
static int g_temp_points[] = { -20, -10, 0, 25, 40, 60 };

static int get_battery_data_by_id_volt(unsigned int id_index,
	unsigned int id_voltage)
{
	if (id_index >= g_bat_data_size)
		return -EINVAL;

	if ((id_voltage > g_p_data[id_index]->id_identify_min) &&
		(id_voltage <= g_p_data[id_index]->id_identify_max)) {
		if ((id_voltage < g_p_data[id_index]->id_voltage_min) ||
		(id_voltage > g_p_data[id_index]->id_voltage_max))
#ifdef CONFIG_HUAWEI_DSM
			power_dsm_report_format_dmd(POWER_DSM_BATTERY_DETECT,
				DSM_BATTERY_DETECT_ERROR_NO,
				"Battery id voltage:%d is out of normal range:[%d~%d],identify range:[%d~%d]!\n",
				id_voltage,
				g_p_data[id_index]->id_voltage_min,
				g_p_data[id_index]->id_voltage_max,
				g_p_data[id_index]->id_identify_min,
				g_p_data[id_index]->id_identify_max);
#else
			bat_data_info("Battery id voltage:%d is out of normal range:[%d~%d],identify range:[%d~%d]!\n",
				id_voltage,
				g_p_data[id_index]->id_voltage_min,
				g_p_data[id_index]->id_voltage_max,
				g_p_data[id_index]->id_identify_min,
				g_p_data[id_index]->id_identify_max);
#endif
		return 0;
	}
	return -EINVAL;
}

static int get_battery_data_by_id_sn(unsigned int id_index)
{
#ifdef CONFIG_HUAWEI_BATTERY_INFORMATION
	int ret;
	unsigned char id_sn[ID_SN_SIZE] = {0};

	if (id_index >= g_bat_data_size)
		return -EINVAL;

	ret = get_battery_type(id_sn, ID_SN_SIZE);
	if (ret) {
		bat_data_err("get id_sn from ic fail!\n");
		return -EINVAL;
	}
	bat_data_info("id_sn from ic is %s\n", id_sn);
	if (!strncmp(g_p_data[id_index]->id_sn, id_sn, strlen(id_sn)))
		return 0;
	return -EINVAL;
#else
	bat_data_err("has no CONFIG_BATTERY_INFORMATION\n");
	return -EINVAL;
#endif
}

struct coul_battery_data *get_battery_data(unsigned int id_voltage)
{
	unsigned int i;
	int ret;

	if (!g_bat_param_status) {
		bat_data_err("battery param is invalid\n");
		return NULL;
	}

	for (i = 0; i < g_bat_data_size; i++) {
		if (!strncmp(g_p_data[i]->identify_type,
			BATT_IDENTIFY_BY_VOLT, strlen(BATT_IDENTIFY_BY_VOLT))) {
			ret = get_battery_data_by_id_volt(i, id_voltage);
			if (!ret)
				break;
		} else if (!strncmp(g_p_data[i]->identify_type,
			BATT_IDENTIFY_BY_SN, strlen(BATT_IDENTIFY_BY_SN))) {
			ret = get_battery_data_by_id_sn(i);
			if (!ret)
				break;
		} else {
			bat_data_err("batt_identify_type error\n");
		}
	}

	if (i == g_bat_data_size) {
		if ((g_default_batt_id_index >= 0) &&
			((unsigned int)g_default_batt_id_index <
						g_bat_data_size)) {
			i = g_default_batt_id_index;
			bat_data_info("use default battery of index %d\n",
					g_default_batt_id_index);
		} else {
			i = 0;
			if (!strstr(saved_command_line,
				"androidboot.swtype=factory"))
#ifdef CONFIG_HUAWEI_DSM
				power_dsm_report_format_dmd(
					POWER_DSM_BATTERY_DETECT,
					DSM_BATTERY_DETECT_ERROR_NO,
					"Battery id is invalid, Use the default battery params!\n");
#else
				bat_data_info("Battery id is invalid, Use the default battery params!\n");
#endif
			g_p_data[i]->id_status = BAT_ID_INVALID;
		}
	}
	bat_info("current battery name is %s\n", g_p_data[i]->batt_brand);

	return g_p_data[i];
}

static void get_property_index(struct device_node *np,
	struct coul_battery_data *pdat, int i, int j)
{
	int ret;

	if (pdat->vol_dec1 > 0) {
		ret = of_property_read_u32_index(np, "pc_temp_ocv_ocv1",
			j * TEMP_SAMPLING_POINTS + i,
			(unsigned int *)(&(pdat->pc_temp_ocv_lut1->ocv[j][i])));
		if (ret)
			bat_data_err("get pc_temp_ocv_ocv1[%d] failed\n",
				j * TEMP_SAMPLING_POINTS + i);
	}
	if (pdat->vol_dec2 > 0) {
		ret = of_property_read_u32_index(np, "pc_temp_ocv_ocv2",
			j * TEMP_SAMPLING_POINTS + i,
			(unsigned int *)(&(pdat->pc_temp_ocv_lut2->ocv[j][i])));
		if (ret)
			bat_data_err("get pc_temp_ocv_ocv2[%d] failed\n",
				j * TEMP_SAMPLING_POINTS + i);
	}
	if (pdat->vol_dec3 > 0) {
		ret = of_property_read_u32_index(np, "pc_temp_ocv_ocv3",
			j * TEMP_SAMPLING_POINTS + i,
			(unsigned int *)(&(pdat->pc_temp_ocv_lut3->ocv[j][i])));
		if (ret)
			bat_data_err("get pc_temp_ocv_ocv3[%d] failed\n",
				j * TEMP_SAMPLING_POINTS + i);
	}
}

static void get_temp_ocv_index(struct device_node *np,
	struct coul_battery_data *pdat)
{
	int i, j;

	for (i = 0; i < pdat->pc_temp_ocv_lut0->cols; i++) { /* 6 */
		for (j = 0; j < pdat->pc_temp_ocv_lut0->rows; j++) /* 29 */
			get_property_index(np, pdat, i, j);
	}
}

static void get_vol_property(struct device_node *np,
	struct coul_battery_data *pdat)
{
	int ret;

	ret = of_property_read_u32(np, "vol_dec1",
		(unsigned int *)(&(pdat->vol_dec1)));
	if (ret) {
		bat_data_err("there is no vol_dec1 config!\n");
		pdat->vol_dec1 = 0;
	}
	ret = of_property_read_u32(np, "vol_dec2",
		(unsigned int *)(&(pdat->vol_dec2)));
	if (ret) {
		bat_data_err("there is no vol_dec2 config!\n");
		pdat->vol_dec2 = 0;
	}
	ret = of_property_read_u32(np, "vol_dec3",
		(unsigned int *)(&(pdat->vol_dec3)));
	if (ret) {
		bat_data_err("there is no vol_dec3 config!\n");
		pdat->vol_dec3 = 0;
	}
}

static int get_age_para(struct device_node *np,
	struct coul_battery_data *pdat)
{
	int i;

	if (!np || !pdat || !pdat->pc_temp_ocv_lut1 ||
		!pdat->pc_temp_ocv_lut2 || !pdat->pc_temp_ocv_lut3 ||
		!pdat->pc_temp_ocv_lut0)
		return -1;

	pdat->pc_temp_ocv_lut1->rows = pdat->pc_temp_ocv_lut0->rows;
	pdat->pc_temp_ocv_lut2->rows = pdat->pc_temp_ocv_lut0->rows;
	pdat->pc_temp_ocv_lut3->rows = pdat->pc_temp_ocv_lut0->rows;

	pdat->pc_temp_ocv_lut1->cols = pdat->pc_temp_ocv_lut0->cols;
	pdat->pc_temp_ocv_lut2->cols = pdat->pc_temp_ocv_lut0->cols;
	pdat->pc_temp_ocv_lut3->cols = pdat->pc_temp_ocv_lut0->cols;

	for (i = 0; i < pdat->pc_temp_ocv_lut0->cols; i++) {
		pdat->pc_temp_ocv_lut1->temp[i] = g_temp_points[i];
		pdat->pc_temp_ocv_lut2->temp[i] = g_temp_points[i];
		pdat->pc_temp_ocv_lut3->temp[i] = g_temp_points[i];
	}

	for (i = 0; i < pdat->pc_temp_ocv_lut0->rows - 1; i++) {
		pdat->pc_temp_ocv_lut1->percent[i] =
			pdat->pc_temp_ocv_lut0->percent[i];
		pdat->pc_temp_ocv_lut2->percent[i] =
			pdat->pc_temp_ocv_lut0->percent[i];
		pdat->pc_temp_ocv_lut3->percent[i] =
			pdat->pc_temp_ocv_lut0->percent[i];
		bat_info("pc_temp_ocv_percent[%d] is %d\n",
			i, pdat->pc_temp_ocv_lut0->percent[i]);
	}

	get_vol_property(np, pdat);
	get_temp_ocv_index(np, pdat);
	return 0;
}

static int get_pc_sf_lut_dts(struct device_node *np,
	struct coul_battery_data *pdat)
{
	int ret;

	ret = of_property_read_u32(np, "pc_sf_rows",
		(unsigned int *)(&(pdat->pc_sf_lut->rows)));
	if (ret) {
		bat_data_err("get pc_sf_rows failed\n");
		return -EINVAL;
	}
	bat_info("pc_sf_rows is %d\n", pdat->pc_sf_lut->rows);
	ret = of_property_read_u32(np, "pc_sf_cols",
		(unsigned int *)(&(pdat->pc_sf_lut->cols)));
	if (ret) {
		bat_data_err("get pc_sf_cols failed\n");
		return -EINVAL;
	}
	bat_info("pc_sf_cols is %d\n", pdat->pc_sf_lut->cols);
	ret = of_property_read_u32(np, "pc_sf_row_entries",
		(unsigned int *)(&(pdat->pc_sf_lut->row_entries[0])));
	if (ret) {
		bat_data_err("get pc_sf_row_entries failed\n");
		return -EINVAL;
	}
	bat_info("pc_sf_row_entries is %d\n",
		pdat->pc_sf_lut->row_entries[0]);
	ret = of_property_read_u32(np, "pc_sf_percent",
		(unsigned int *)(&(pdat->pc_sf_lut->percent[0])));
	if (ret) {
		bat_data_err("get pc_sf_percent failed\n");
		return -EINVAL;
	}
	bat_info("pc_sf_percent is %d\n", pdat->pc_sf_lut->percent[0]);
	ret = of_property_read_u32(np, "pc_sf_sf",
		(unsigned int *)(&(pdat->pc_sf_lut->sf[0][0])));
	if (ret) {
		bat_data_err("get pc_sf_sf failed\n");
		return -EINVAL;
	}
	bat_info("pc_sf_sf is %d\n", pdat->pc_sf_lut->sf[0][0]);
	return 0;
}

static int get_fcc_sf_dts(struct device_node *np,
	struct coul_battery_data *pdat)
{
	int i, ret;

	ret = of_property_read_u32(np, "fcc_sf_cols",
		(unsigned int *)(&(pdat->fcc_sf_lut->cols)));
	if (ret) {
		bat_data_err("get fcc_sf_cols failed\n");
		return -EINVAL;
	}
	bat_info("fcc_sf_cols is %d\n", pdat->fcc_sf_lut->cols);
	for (i = 0; i < pdat->fcc_sf_lut->cols; i++) {
		ret = of_property_read_u32_index(np, "fcc_sf_x",
			i, (unsigned int *)(&(pdat->fcc_sf_lut->x[i])));
		if (ret) {
			bat_data_err("get fcc_sf_x[%d] failed\n", i);
			return -EINVAL;
		}
		bat_info("fcc_sf_x[%d] is %d\n", i, pdat->fcc_sf_lut->x[i]);
		ret = of_property_read_u32_index(np, "fcc_sf_y",
			i, (unsigned int *)(&(pdat->fcc_sf_lut->y[i])));
		if (ret) {
			bat_data_err("get fcc_sf_y[%d] failed\n", i);
			return -EINVAL;
		}
		bat_info("fcc_sf_y[%d] is %d\n", i, pdat->fcc_sf_lut->y[i]);
	}
	/* pc_sf_lut */
	ret = get_pc_sf_lut_dts(np, pdat);
	if (ret) {
		bat_data_err("get get_pc_sf_lut_dts failed\n");
		return -EINVAL;
	}

	return 0;
}

static int get_id_volt_dat(struct device_node *np,
	struct coul_battery_data *pdat)
{
	int ret;

	ret = of_property_read_u32(np, "id_identify_min",
		(unsigned int *)(&(pdat->id_identify_min)));
	if (ret) {
		bat_data_err("get id_identify_min failed\n");
		return -EINVAL;
	}
	bat_info("id_identify_min is %d\n", pdat->id_identify_min);

	ret = of_property_read_u32(np, "id_identify_max",
		(unsigned int *)(&(pdat->id_identify_max)));
	if (ret) {
		bat_data_err("get id_identify_max failed\n");
		return -EINVAL;
	}
	bat_info("id_identify_max is %d\n", pdat->id_identify_max);

	ret = of_property_read_u32(np, "id_voltage_min",
		(unsigned int *)(&(pdat->id_voltage_min)));
	if (ret) {
		bat_data_err("get id_voltage_min failed\n");
		return -EINVAL;
	}
	bat_info("id_voltage_min is %d\n", pdat->id_voltage_min);

	ret = of_property_read_u32(np, "id_voltage_max",
		(unsigned int *)(&(pdat->id_voltage_max)));
	if (ret) {
		bat_data_err("get id_voltage_max failed\n");
		return -EINVAL;
	}
	bat_info("id_voltage_max is %d\n", pdat->id_voltage_max);
	return 0;
}

static int get_id_sn_dat(struct device_node *np,
	struct coul_battery_data *pdat)
{
	int ret;

	ret = of_property_read_string(np,
		"id_sn", (const char **)(&pdat->id_sn));
	if (ret) {
		bat_data_err("get id_sn failed\n");
		return -EINVAL;
	}
	bat_info("id_sn is %s\n", pdat->id_sn);
	return 0;
}

static int get_dat_decress_para(struct device_node *np,
	struct coul_battery_data *pdat)
{
	int ret;

	if (!np || !pdat)
		return -EINVAL;
	if (strstr(saved_command_line,
			"batt_decress_flag=true")) {
		ret = of_property_read_u32(np, "fcc_decress", &(pdat->fcc));
		if (ret) {
			bat_data_err("get fcc decress failed\n");
			return -EINVAL;
		}
		ret = of_property_read_u32(np, "vbat_max_decress",
			(unsigned int *)(&(pdat->vbatt_max)));
		if (ret) {
			bat_data_err("get vbat_max decress failed\n");
			return -EINVAL;
		}
	}
	return 0;
}

static int get_decress_fcc_temp(struct device_node *np,
	struct coul_battery_data *pdat, int index)
{
	int ret;

	if (!np || !pdat)
		return -EINVAL;
	if (strstr(saved_command_line, "batt_decress_flag=true")) {
		ret = of_property_read_u32_index(np, "fcc_temp_decress",
			index, (unsigned int *)(&(pdat->fcc_temp_lut->y[index])));
		if (ret) {
			bat_data_err("get fcc_temp decress[%d] failed\n", index);
			return -EINVAL;
		}
	}
	return 0;
}

static int get_decress_ocv_tbl(struct device_node *np,
	struct coul_battery_data *pdat, int row, int col)
{
	int ret;

	if (!np || !pdat)
		return -EINVAL;
	if (strstr(saved_command_line, "batt_decress_flag=true")) {
		ret = of_property_read_u32_index(np, "pc_temp_ocv_ocv0",
			row * TEMP_SAMPLING_POINTS + col, (unsigned int *)(
			&(pdat->pc_temp_ocv_lut0->ocv[row][col]))); /* cols */
		if (ret) {
			bat_data_err("get pc_temp_ocv_ocv drcress[%d] failed\n",
				row * TEMP_SAMPLING_POINTS + col);
			return -EINVAL;
		}
	}
	return 0;
}

static int get_decress_segment_para(struct device_node *np,
	int index, const char **data_string)
{
	int ret;

	if (!np || !data_string)
		return -EINVAL;
	if (strstr(saved_command_line, "batt_decress_flag=true")) {
		ret = of_property_read_string_index(np,
			"segment_para_decress", index, data_string);
		if (ret) {
			bat_data_err("get segment_para_decress failed\n");
			return -EINVAL;
		}
	}
	return 0;
}

static int get_dat_basic_para(struct device_node *np,
	struct coul_battery_data *pdat)
{
	int ret;

	ret = of_property_read_u32(np, "fcc", &(pdat->fcc));
	if (ret) {
		bat_data_err("get fcc failed\n");
		return -EINVAL;
	}
	bat_info("fcc is %u\n", pdat->fcc);

	ret = of_property_read_string(np, "identify_type",
		(const char **)(&pdat->identify_type));
	if (ret) {
		bat_data_err("get batt_identify_type failed!\n");
		return -EINVAL;
	}
	bat_info("identify_type is %s\n", pdat->identify_type);
	if (!strncmp(pdat->identify_type,
		BATT_IDENTIFY_BY_VOLT, strlen(BATT_IDENTIFY_BY_VOLT))) {
		ret = get_id_volt_dat(np, pdat);
	} else if (!strncmp(pdat->identify_type,
		BATT_IDENTIFY_BY_SN, strlen(BATT_IDENTIFY_BY_SN))) {
		ret = get_id_sn_dat(np, pdat);
	} else {
		bat_data_err("batt_identify_type error\n");
		return -EINVAL;
	}
	if (ret) {
		bat_data_err("get batt_identify_para failed!\n");
		return -EINVAL;
	}
	ret = of_property_read_u32(np, "default_rbatt_mohm",
		(unsigned int *)(&(pdat->default_rbatt_mohm)));
	if (ret) {
		bat_data_err("get default_rbatt_mohm failed\n");
		return -EINVAL;
	}
	bat_info("default_rbatt_mohm is %d\n", pdat->default_rbatt_mohm);
	/* vbat_max */
	ret = of_property_read_u32(np, "vbat_max", (unsigned int *)(&(pdat->vbatt_max)));
	if (ret) {
		bat_data_err("get vbat_max failed\n");
		return -EINVAL;
	}
	bat_info("vbat_max is %d\n", pdat->vbatt_max);
	/* ifull */
	ret = of_property_read_u32(np, "ifull", (unsigned int *)(&(pdat->ifull)));
	if (ret) {
		pdat->ifull = DEFAULT_IFULL_SET;
		bat_data_err("get ifull failed,using DEFAULT_IFULL_SET\n");
	}
	bat_info("ifull is %d\n", pdat->ifull);
	return 0;
}

static int get_dat_temp_para(struct device_node *np,
	struct coul_battery_data *pdat)
{
	int ret, i, array_len;
	const char *chrg_data_string = NULL;

	array_len = of_property_count_strings(np, "temp_para");
	if ((array_len <= 0) || (array_len % TEMP_PARA_TOTAL != 0) ||
		(array_len > TEMP_PARA_LEVEL * TEMP_PARA_TOTAL)) {
		bat_data_err("temp_para is invaild,please check temp_para number!!\n");
		return -EINVAL;
	}
	pdat->chrg_para->temp_len = array_len;
	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np,
			"temp_para", i, &chrg_data_string);
		if (ret) {
			bat_data_err("get temp_para failed\n");
			return -EINVAL;
		}
		ret = kstrtol(chrg_data_string, DEC,
			&pdat->chrg_para->temp_data[
			i / (TEMP_PARA_TOTAL)][i % (TEMP_PARA_TOTAL)]); /* max size */
		if (ret < 0)
			bat_info("convert temp_para data failed\n");
		else
			bat_info("chrg_para->temp_data[%d][%d] = %d\n",
				i / (TEMP_PARA_TOTAL), i % (TEMP_PARA_TOTAL),
				pdat->chrg_para->temp_data[
				i / (TEMP_PARA_TOTAL)][i % (TEMP_PARA_TOTAL)]);
	}
	return 0;
}

static int get_dat_vbat_para(struct device_node *np,
	struct coul_battery_data *pdat)
{
	int ret, i, array_len;
	const char *chrg_data_string = NULL;

	array_len = of_property_count_strings(np, "vbat_para");
	if ((array_len <= 0) || (array_len % VOLT_PARA_TOTAL != 0) ||
		(array_len > VOLT_PARA_LEVEL * VOLT_PARA_TOTAL)) {
		bat_data_err("vbat_para is invaild,please check vbat_para number!!\n");
		return -EINVAL;
	}
	pdat->chrg_para->volt_len = array_len;
	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np,
			"vbat_para", i, &chrg_data_string);
		if (ret) {
			bat_data_err("get vbat_para failed\n");
			return -EINVAL;
		}
		ret = kstrtol(chrg_data_string, DEC,
			&pdat->chrg_para->volt_data[
			i / (VOLT_PARA_TOTAL)][i % (VOLT_PARA_TOTAL)]); /* max size */
		if (ret < 0)
			bat_info("convert vbat_para data failed\n");
		else
			bat_info("chrg_para->volt_data[%d][%d] = %d\n",
				i / (VOLT_PARA_TOTAL), i % (VOLT_PARA_TOTAL),
				pdat->chrg_para->volt_data[
				i / (VOLT_PARA_TOTAL)][i % (VOLT_PARA_TOTAL)]);
	}
	return 0;
}

static int get_dat_segment_para(struct device_node *np,
	struct coul_battery_data *pdat)
{
	int ret, i, array_len;
	const char *chrg_data_string = NULL;

	array_len = of_property_count_strings(np, "segment_para");
	if ((array_len <= 0) || (array_len % SEGMENT_PARA_TOTAL != 0) ||
		(array_len > SEGMENT_PARA_LEVEL * SEGMENT_PARA_TOTAL)) {
		bat_data_err("segment_para is invaild,please check segment_para number!\n");
		return -EINVAL;
	}
	pdat->chrg_para->segment_len = array_len;
	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np,
			"segment_para", i, &chrg_data_string);
		if (ret) {
			bat_data_err("get segment_para failed\n");
			return -EINVAL;
		}
		ret = get_decress_segment_para(np, i, &chrg_data_string);
		bat_data_info("get_decress_segment_para res %d", ret);
		ret = kstrtol(chrg_data_string, DEC,
			&pdat->chrg_para->segment_data[
			i / (SEGMENT_PARA_TOTAL)][i % (SEGMENT_PARA_TOTAL)]); /* max size */
		if (ret < 0)
			bat_info("convert segment_data failed\n");
		else
			bat_info("chrg_para->segment_data[%d][%d] = %d\n",
				i / (SEGMENT_PARA_TOTAL), i % (SEGMENT_PARA_TOTAL),
				pdat->chrg_para->segment_data[i / (SEGMENT_PARA_TOTAL)][
					i % (SEGMENT_PARA_TOTAL)]);
	}
	return 0;
}

static int get_dat_fcc_temp(struct device_node *np,
	struct coul_battery_data *pdat)
{
	int ret, i;

	pdat->fcc_temp_lut->cols = TEMP_SAMPLING_POINTS;
	for (i = 0; i < pdat->fcc_temp_lut->cols; i++) {
		pdat->fcc_temp_lut->x[i] = g_temp_points[i];
		ret = of_property_read_u32_index(np, "fcc_temp",
			i, (unsigned int *)(&(pdat->fcc_temp_lut->y[i])));
		if (ret) {
			bat_data_err("get fcc_temp[%d] failed\n", i);
			return -EINVAL;
		}
		ret = get_decress_fcc_temp(np, pdat, i);
		bat_info("get_decress_fcc_temp res %d", ret);
		bat_info("fcc_temp[%d] is %d\n",
				i, pdat->fcc_temp_lut->y[i]);
	}
	return 0;
}

static int get_dat_rbat_sf(struct device_node *np,
	struct coul_battery_data *pdat)
{
	int ret, i, j;

	ret = of_property_read_u32(np, "rbatt_sf_rows",
		(unsigned int *)(&(pdat->rbatt_sf_lut->rows)));
	if (ret) {
		bat_data_err("get rbatt_sf_rows failed\n");
		return -EINVAL;
	}
	bat_info("rbatt_sf_rows is %d\n", pdat->rbatt_sf_lut->rows);
	ret = of_property_read_u32(np, "rbatt_sf_cols",
		(unsigned int *)(&(pdat->rbatt_sf_lut->cols)));
	if (ret) {
		bat_data_err("get rbatt_sf_cols failed\n");
		return -EINVAL;
	}
	bat_info("rbatt_sf_cols is %d\n", pdat->rbatt_sf_lut->cols);
	for (i = 0; i < pdat->rbatt_sf_lut->rows; i++) {
		ret = of_property_read_u32_index(np, "rbatt_sf_percent", i,
			(unsigned int *)(&(pdat->rbatt_sf_lut->percent[i])));
		if (ret) {
			bat_data_err("get rbatt_sf_percent[%d] failed\n", i);
			return -EINVAL;
		}
		bat_info("rbatt_sf_percent[%d] is %d\n",
			i, pdat->rbatt_sf_lut->percent[i]);
	}
	for (i = 0; i < pdat->rbatt_sf_lut->cols; i++)
		pdat->rbatt_sf_lut->row_entries[i] = g_temp_points[i];

	for (i = 0; i < pdat->rbatt_sf_lut->cols; i++) { /* 6 */
		for (j = 0; j < pdat->rbatt_sf_lut->rows; j++) { /* 28 */
			ret = of_property_read_u32_index(np,
			  "rbatt_sf_sf", j * TEMP_SAMPLING_POINTS + i, /* cols */
			  (unsigned int *)(&(pdat->rbatt_sf_lut->sf[j][i])));
			if (ret) {
				bat_data_err("get rbatt_sf_sf[%d] failed\n",
					j * TEMP_SAMPLING_POINTS + i);
				return -EINVAL;
			}
			bat_info("rbatt_sf_sf[%d] is %d\n",
				j * TEMP_SAMPLING_POINTS + i,
				pdat->rbatt_sf_lut->sf[j][i]);
		}
	}
	return 0;
}

static int get_dat_pc_temp_ocv(struct device_node *np,
	struct coul_battery_data *pdat)
{
	int ret, i, j;

	ret = of_property_read_u32(np, "pc_temp_ocv_rows",
		(unsigned int *)(&(pdat->pc_temp_ocv_lut0->rows)));
	if (ret) {
		bat_data_err("get pc_temp_ocv_rows failed\n");
		return -EINVAL;
	}
	bat_info("pc_temp_ocv_rows is %d\n", pdat->pc_temp_ocv_lut->rows);
	ret = of_property_read_u32(np, "pc_temp_ocv_cols",
		(unsigned int *)(&(pdat->pc_temp_ocv_lut0->cols)));
	if (ret) {
		bat_data_err("get pc_temp_ocv_cols failed\n");
		return -EINVAL;
	}
	bat_info("pc_temp_ocv_cols is %d\n", pdat->pc_temp_ocv_lut0->cols);
	for (i = 0; i < pdat->pc_temp_ocv_lut0->rows - 1; i++) {
		ret = of_property_read_u32_index(np,
		 "pc_temp_ocv_percent", i,
		 (unsigned int *)(&(pdat->pc_temp_ocv_lut0->percent[i])));
		if (ret) {
			bat_data_err("get pc_temp_ocv_percent[%d] failed\n", i);
			return -EINVAL;
		}
		bat_info("pc_temp_ocv_percent[%d] is %d\n",
			i, pdat->pc_temp_ocv_lut0->percent[i]);
	}
	for (i = 0; i < pdat->pc_temp_ocv_lut0->cols; i++)
		pdat->pc_temp_ocv_lut0->temp[i] = g_temp_points[i];

	for (i = 0; i < pdat->pc_temp_ocv_lut0->cols; i++) { /* 6 */
		for (j = 0; j < pdat->pc_temp_ocv_lut0->rows; j++) { /* 29 */
			ret = of_property_read_u32_index(np, "pc_temp_ocv_ocv",
			  j * TEMP_SAMPLING_POINTS + i, (unsigned int *)(
			  &(pdat->pc_temp_ocv_lut0->ocv[j][i]))); /* cols */
			if (ret) {
				bat_data_err("get pc_temp_ocv_ocv[%d] failed\n",
					j * TEMP_SAMPLING_POINTS + i);
				return -EINVAL;
			}
			ret = get_decress_ocv_tbl(np, pdat, j, i);
			bat_info("get_decress_ocv_tbl res %d", ret);
			bat_info("rbatt_sf_sf[%d] is %d\n",
				j * TEMP_SAMPLING_POINTS + i,
				pdat->pc_temp_ocv_lut0->ocv[j][i]);
		}
	}
	return 0;
}

static int get_dat(struct device_node *np, struct coul_battery_data *pdat)
{
	int ret;

	ret = get_dat_basic_para(np, pdat);
	if (ret)
		return -EINVAL;
	/* temp_para */
	ret = get_dat_temp_para(np, pdat);
	if (ret)
		return -EINVAL;
	/* vbat_para */
	ret = get_dat_vbat_para(np, pdat);
	if (ret)
		return -EINVAL;
	/* segment_para */
	ret = get_dat_segment_para(np, pdat);
	if (ret)
		return -EINVAL;
	/* batt_brand */
	ret = of_property_read_string(
		np, "batt_brand", (const char **)(&(pdat->batt_brand)));
	if (ret) {
		bat_data_err("get batt_brand failed\n");
		return -EINVAL;
	}
	bat_info("batt_brand is %s\n", pdat->batt_brand);
	/* fcc_temp */
	ret = get_dat_fcc_temp(np, pdat);
	if (ret)
		return -EINVAL;
	/* decress para */
	ret = get_dat_decress_para(np, pdat);
	bat_data_info("get decress para from dts res %d\n", ret);
	/* fcc_sf */
	ret = get_fcc_sf_dts(np, pdat);
	if (ret) {
		bat_data_err("get_fcc_sf_dts failed\n");
		return -EINVAL;
	}
	/* rbat_sf */
	ret = get_dat_rbat_sf(np, pdat);
	if (ret)
		return -EINVAL;
	/* pc_temp_ocv */
	ret = get_dat_pc_temp_ocv(np, pdat);
	if (ret)
		return -EINVAL;
	get_age_para(np, pdat);

	return ret;
}

static void get_mem_extra(struct coul_battery_data *pdat)
{
	if (!pdat)
		return;
	pdat->pc_temp_ocv_lut1 =
		kzalloc(sizeof(*pdat->pc_temp_ocv_lut1), GFP_KERNEL);
	if (!pdat->pc_temp_ocv_lut1)
		return;
	pdat->pc_temp_ocv_lut2 =
		kzalloc(sizeof(*pdat->pc_temp_ocv_lut2), GFP_KERNEL);
	if (!pdat->pc_temp_ocv_lut2) {
		kfree(pdat->pc_temp_ocv_lut1);
		pdat->pc_temp_ocv_lut1 = NULL;
		return;
	}
	pdat->pc_temp_ocv_lut3 =
		kzalloc(sizeof(*pdat->pc_temp_ocv_lut3), GFP_KERNEL);
	if (!pdat->pc_temp_ocv_lut3) {
		kfree(pdat->pc_temp_ocv_lut1);
		kfree(pdat->pc_temp_ocv_lut2);
		pdat->pc_temp_ocv_lut1 = NULL;
		pdat->pc_temp_ocv_lut2 = NULL;
		return;
	}
}

static int get_mem_pcsf(struct coul_battery_data *pdat)
{
	pdat->pc_sf_lut = kzalloc(sizeof(*pdat->pc_sf_lut), GFP_KERNEL);
	if (!pdat->pc_sf_lut)
		goto get_mempc_fail_0;

	pdat->rbatt_sf_lut = kzalloc(sizeof(*pdat->rbatt_sf_lut), GFP_KERNEL);
	if (!pdat->rbatt_sf_lut)
		goto get_mempc_fail_1;

	pdat->chrg_para = kzalloc(sizeof(*pdat->chrg_para), GFP_KERNEL);
	if (!pdat->chrg_para)
		goto get_mempc_fail_2;

	return 0;
get_mempc_fail_2:
	kfree(pdat->rbatt_sf_lut);
get_mempc_fail_1:
	kfree(pdat->pc_sf_lut);
get_mempc_fail_0:
	return -ENOMEM;
}

static int get_mem(struct coul_battery_data **p)
{
	struct coul_battery_data *pdat = NULL;
	int ret;

	pdat = kzalloc(sizeof(*pdat), GFP_KERNEL);
	if (!pdat)
		return -ENOMEM;

	pdat->fcc_temp_lut = kzalloc(sizeof(*pdat->fcc_temp_lut), GFP_KERNEL);
	if (!pdat->fcc_temp_lut)
		goto get_mem_fail_0;

	pdat->fcc_sf_lut = kzalloc(sizeof(*pdat->fcc_sf_lut), GFP_KERNEL);
	if (!pdat->fcc_sf_lut)
		goto get_mem_fail_1;

	pdat->pc_temp_ocv_lut0 =
		kzalloc(sizeof(*pdat->pc_temp_ocv_lut0), GFP_KERNEL);
	if (!pdat->pc_temp_ocv_lut0)
		goto get_mem_fail_2;

	pdat->pc_temp_ocv_lut = pdat->pc_temp_ocv_lut0;

	ret = get_mem_pcsf(pdat);
	if (ret)
		goto get_mem_fail_3;

	get_mem_extra(pdat);
	*p = pdat;
	return 0;
get_mem_fail_3:
	pdat->pc_temp_ocv_lut = NULL;
	kfree(pdat->pc_temp_ocv_lut0);
get_mem_fail_2:
	kfree(pdat->fcc_sf_lut);
get_mem_fail_1:
	kfree(pdat->fcc_temp_lut);
get_mem_fail_0:
	kfree(pdat);
	return -ENOMEM;
}

static void free_mem(struct coul_battery_data **p)
{
	struct coul_battery_data *pdat = *p;

	if (!pdat) {
		bat_data_err("pointer pd is already NULL\n");
		return;
	}

	kfree(pdat->fcc_temp_lut);
	pdat->fcc_temp_lut = NULL;

	kfree(pdat->fcc_sf_lut);
	pdat->fcc_sf_lut = NULL;

	pdat->pc_temp_ocv_lut = NULL;
	kfree(pdat->pc_temp_ocv_lut0);
	pdat->pc_temp_ocv_lut0 = NULL;

	kfree(pdat->pc_sf_lut);
	pdat->pc_sf_lut = NULL;

	kfree(pdat->rbatt_sf_lut);
	pdat->rbatt_sf_lut = NULL;

	kfree(pdat->chrg_para);
	pdat->chrg_para = NULL;

	kfree(pdat->pc_temp_ocv_lut1);
	pdat->pc_temp_ocv_lut1 = NULL;

	kfree(pdat->pc_temp_ocv_lut2);
	pdat->pc_temp_ocv_lut2 = NULL;

	kfree(pdat->pc_temp_ocv_lut3);
	pdat->pc_temp_ocv_lut3 = NULL;
	kfree(pdat);
	*p = NULL;
}

static int get_p_data(struct platform_device *pdev, struct device_node *np)
{
	int retval;
	unsigned int i;
	struct device_node *bat_node = NULL;

	/* alloc memory to store pointers(point to battery data) */
	g_p_data = kzalloc(g_bat_data_size *
		sizeof(struct coul_battery_data *), GFP_KERNEL);
	if (!g_p_data)
		return -1;

	for (i = 0; i < g_bat_data_size; ++i) {
		retval = get_mem(&(g_p_data[i]));
		if (retval) {
			bat_data_err("get_mem[%d] failed\n", i);
			goto free_p_data;
		}
		bat_node = of_parse_phandle(np, "batt_name", i);
		if (!bat_node) {
			bat_data_err("get bat_node failed\n");
			goto free_p_data;
		}
		retval = get_dat(bat_node, g_p_data[i]);
		if (retval) {
			bat_data_err("get_dat[%d] failed\n", i);
			goto free_p_data;
		}
	}
	platform_set_drvdata(pdev, g_p_data);
	return 0;

free_p_data:
	if (g_p_data) {
		for (i = 0; i < g_bat_data_size; ++i)
			free_mem(&(g_p_data[i]));

		kfree(g_p_data);
	}
	g_p_data = NULL;
	return -EINVAL;
}

static int battery_data_probe(struct platform_device *pdev)
{
	int retval;
	unsigned int i;
	struct device_node *np = NULL;

	/* get device node for battery module */
	g_bat_param_status = 0;
	np = pdev->dev.of_node;
	if (!np) {
		bat_data_err("get device node failed\n");
		return -EINVAL;
	}

	/* get numeber of types */
	for (i = 0;; ++i) {
		if (!of_parse_phandle(np, "batt_name", i))
			break;
	}
	if (!i) {
		bat_data_err("bat_data_size is zero\n");
		return -EINVAL;
	}
	g_bat_data_size = i;
	bat_data_info("bat_data_size = %u\n", g_bat_data_size);

	retval = get_p_data(pdev, np);
	if (retval)
		return -EINVAL;

	g_bat_param_status = 1;
	bat_data_info("probe ok\n");
	return 0;
}

static int battery_data_remove(struct platform_device *pdev)
{
	unsigned int i;
	struct coul_battery_data **di = platform_get_drvdata(pdev);

	if (di) {
		for (i = 0; i < g_bat_data_size; ++i)
			free_mem(&(di[i]));

		kfree(di);
	}
	g_p_data = NULL;
	return 0;
}

static const struct of_device_id bat_match_table[] = {
	{
		.compatible = "hisilicon,battery",
		.data = NULL,
	},
	{
	},
};

static struct platform_driver bat_driver = {
	.probe = battery_data_probe,
	.remove = battery_data_remove,
	.driver = {
		.name = "battery",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(bat_match_table),
	},
};

int __init battery_init(void)
{
	return platform_driver_register(&bat_driver);
}

void __exit battery_exit(void)
{
	platform_driver_unregister(&bat_driver);
}

fs_initcall(battery_init);
module_exit(battery_exit);
MODULE_DESCRIPTION("battery module driver");
MODULE_LICENSE("GPL");

