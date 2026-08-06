/*
 * ntc_tm_table.c
 *
 * thermal table module
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
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

#include <linux/err.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/input.h>
#include <linux/time.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/syscalls.h>
#include <linux/mutex.h>
#include <linux/io.h>
#include <linux/hisi/hisi_adc.h>
#include "peripheral_tm.h"

#ifdef CONFIG_HKADC_MODEM
#include "modem_hkadc_temp.h"
#endif

/* in order to resolve divisor less than zero */
#define PRORATE_OF_INIT	1000
#define NCP_GENERAL1_NUM	34
#define ROOM_DEFAULT_TEMP	25000

enum ntc_tm_table_list {
	THERMAL_ADC2TEMP_FLAG = 0,
	THERMAL_PA_VOLT2TEMP_FLAG = 1,
	THERMAL_SOC_VOLT2TEMP_FLAG = 2
};

/* DCXO: {volt(mv), temp(C)} */
int g_thermal_hkadc_volt_temp_table[][2] = {
	{51, 120}, {59, 115}, {68, 110}, {79, 105}, {92, 100}, {107, 95},
	{125, 90}, {146, 85}, {172, 80}, {201, 75}, {237, 70}, {278, 65},
	{327, 60}, {383, 55}, {448, 50}, {522, 45}, {605, 40}, {696, 35},
	{795, 30}, {900, 25}, {1007, 20}, {1114, 15}, {1218, 10}, {1316, 5},
	{1405, 0}, {1484, -5}, {1552, -10}, {1608, -15}, {1655, -20}, {1692, -25},
	{1720, -30}, {1742, -35}
};

int g_thermal_pa_volt_temp_table[][2] = {
	{44, NTC_TEMP_MAX_VALUE}, {46, 124}, {47, 123}, {48, 122}, {50, 121}, {51, 120},
	{53, 119}, {54, 118}, {56, 117}, {57, 116}, {59, 115}, {61, 114}, {62, 113}, {64, 112},
	{66, 111}, {68, 110}, {70, 109}, {72, 108}, {74, 107}, {77, 106}, {79, 105}, {81, 104},
	{84, 103}, {86, 102}, {89, 101}, {92, 100}, {95, 99}, {98, 98}, {101, 97}, {104, 96},
	{107, 95}, {110, 94}, {114, 93}, {118, 92}, {121, 91}, {125, 90}, {129, 89}, {133, 88},
	{137, 87}, {142, 86}, {146, 85}, {151, 84}, {156, 83}, {161, 82}, {166, 81}, {172, 80},
	{177, 79}, {183, 78}, {189, 77}, {195, 76}, {201, 75}, {208, 74}, {215, 73}, {222, 72},
	{229, 71}, {237, 70}, {245, 69}, {253, 68}, {261, 67}, {269, 66}, {278, 65}, {287, 64},
	{297, 63}, {306, 62}, {316, 61}, {327, 60}, {337, 59}, {348, 58}, {360, 57}, {371, 56},
	{383, 55}, {395, 54}, {408, 53}, {421, 52}, {434, 51}, {448, 50}, {462, 49}, {477, 48},
	{491, 47}, {506, 46}, {522, 45}, {538, 44}, {554, 43}, {571, 42}, {588, 41}, {605, 40},
	{623, 39}, {641, 38}, {659, 37}, {677, 36}, {696, 35}, {716, 34}, {735, 33}, {755, 32},
	{775, 31}, {795, 30}, {816, 29}, {836, 28}, {857, 27}, {878, 26}, {900, 25}, {921, 24},
	{942, 23}, {964, 22}, {985, 21}, {1007, 20}, {1028, 19}, {1050, 18}, {1071, 17}, {1093, 16},
	{1114, 15}, {1135, 14}, {1156, 13}, {1177, 12}, {1197, 11}, {1218, 10}, {1238, 9}, {1258, 8},
	{1277, 7}, {1296, 6}, {1315, 5}, {1334, 4}, {1352, 3}, {1370, 2}, {1387, 1}, {1404, 0},
	{1421, -1}, {1437, -2}, {1453, -3}, {1468, -4}, {1483, -5}, {1498, -6}, {1512, -7}, {1525, -8},
	{1538, -9}, {1551, -10}, {1563, -11}, {1575, -12}, {1586, -13}, {1597, -14}, {1608, -15}, {1618, -16},
	{1627, -17}, {1636, -18}, {1645, -19}, {1654, -20}, {1662, -21}, {1670, -22}, {1677, -23}, {1684, -24},
	{1691, -25}, {1697, -26}, {1703, -27}, {1709, -28}, {1714, -29}, {1719, -30}, {1724, -31}, {1729, -32},
	{1733, -33}, {1738, -34}, {1742, -35}, {1745, -36}, {1749, -37}, {1752, -38}, {1755, -39}, {1758, NTC_TEMP_MIN_VALUE},
};

int g_thermal_soc_volt_temp_table[][2] = {
	{44, NTC_TEMP_MAX_VALUE}, {46, 124}, {47, 123}, {48, 122}, {50, 121}, {51, 120},
	{53, 119}, {54, 118}, {56, 117}, {57, 116}, {59, 115}, {61, 114}, {62, 113}, {64, 112},
	{66, 111}, {68, 110}, {70, 109}, {72, 108}, {74, 107}, {77, 106}, {79, 105}, {81, 104},
	{84, 103}, {86, 102}, {89, 101}, {92, 100}, {95, 99}, {98, 98}, {101, 97}, {104, 96},
	{107, 95}, {110, 94}, {114, 93}, {118, 92}, {121, 91}, {125, 90}, {129, 89}, {133, 88},
	{137, 87}, {142, 86}, {146, 85}, {151, 84}, {156, 83}, {161, 82}, {166, 81}, {172, 80},
	{177, 79}, {183, 78}, {189, 77}, {195, 76}, {201, 75}, {208, 74}, {215, 73}, {222, 72},
	{229, 71}, {237, 70}, {245, 69}, {253, 68}, {261, 67}, {269, 66}, {278, 65}, {287, 64},
	{297, 63}, {306, 62}, {316, 61}, {327, 60}, {337, 59}, {348, 58}, {360, 57}, {371, 56},
	{383, 55}, {395, 54}, {408, 53}, {421, 52}, {434, 51}, {448, 50}, {462, 49}, {477, 48},
	{491, 47}, {506, 46}, {522, 45}, {538, 44}, {554, 43}, {571, 42}, {588, 41}, {605, 40},
	{623, 39}, {641, 38}, {659, 37}, {677, 36}, {696, 35}, {716, 34}, {735, 33}, {755, 32},
	{775, 31}, {795, 30}, {816, 29}, {836, 28}, {857, 27}, {878, 26}, {900, 25}, {921, 24},
	{942, 23}, {964, 22}, {985, 21}, {1007, 20}, {1028, 19}, {1050, 18}, {1071, 17}, {1093, 16},
	{1114, 15}, {1135, 14}, {1156, 13}, {1177, 12}, {1197, 11}, {1218, 10}, {1238, 9}, {1258, 8},
	{1277, 7}, {1296, 6}, {1315, 5}, {1334, 4}, {1352, 3}, {1370, 2}, {1387, 1}, {1404, 0},
	{1421, -1}, {1437, -2}, {1453, -3}, {1468, -4}, {1483, -5}, {1498, -6}, {1512, -7}, {1525, -8},
	{1538, -9}, {1551, -10}, {1563, -11}, {1575, -12}, {1586, -13}, {1597, -14}, {1608, -15}, {1618, -16},
	{1627, -17}, {1636, -18}, {1645, -19}, {1654, -20}, {1662, -21}, {1670, -22}, {1677, -23}, {1684, -24},
	{1691, -25}, {1697, -26}, {1703, -27}, {1709, -28}, {1714, -29}, {1719, -30}, {1724, -31}, {1729, -32},
	{1733, -33}, {1738, -34}, {1742, -35}, {1745, -36}, {1749, -37}, {1752, -38}, {1755, -39}, {1758, NTC_TEMP_MIN_VALUE},
};

/*
 * Function: int change_volt_to_temp()
 * Description: volt to temp
 * Input:  NA
 * Output: NA
 * Return: get temperature
 * Others: NA
 */
static int change_volt_to_temp(int resistance, int index_min, int index_max,
			       int table_list)
{
	int prorate, itemper;
	int resistance_max = 0;
	int resistance_min = 0;
	int temper_top = 0;
	int temper_bottom = 0;

	switch (table_list) {
	case THERMAL_ADC2TEMP_FLAG:
		resistance_min = g_thermal_hkadc_volt_temp_table[index_min][0];
		resistance_max = g_thermal_hkadc_volt_temp_table[index_max][0];
		temper_bottom = g_thermal_hkadc_volt_temp_table[index_min][1];
		temper_top = g_thermal_hkadc_volt_temp_table[index_max][1];
		break;
	case THERMAL_PA_VOLT2TEMP_FLAG:
		resistance_min = g_thermal_pa_volt_temp_table[index_min][0];
		resistance_max = g_thermal_pa_volt_temp_table[index_max][0];
		temper_bottom = g_thermal_pa_volt_temp_table[index_min][1];
		temper_top = g_thermal_pa_volt_temp_table[index_max][1];
		break;
	case THERMAL_SOC_VOLT2TEMP_FLAG:
		resistance_min = g_thermal_soc_volt_temp_table[index_min][0];
		resistance_max = g_thermal_soc_volt_temp_table[index_max][0];
		temper_bottom = g_thermal_soc_volt_temp_table[index_min][1];
		temper_top = g_thermal_soc_volt_temp_table[index_max][1];
		break;
	default:
		pr_err("ADC channel is not exist!\n\r");
		break;
	}

	if ((resistance_max - resistance_min) == 0)
		return ROOM_DEFAULT_TEMP;
	prorate = ((resistance_max - resistance) * PRORATE_OF_INIT) /
		    (resistance_max - resistance_min); /*lint !e414*/
	itemper = (temper_bottom - temper_top) * prorate +
		   temper_top * PRORATE_OF_INIT;
	return itemper;
}

static int temp_2_ntc_linear_value(int cur_temp, int boundary_start_index,
				   int boundary_end_index, int table[][2])
{
	int ntc_linear_start, ntc_linear_end;
	int temp_linear_start, temp_linear_end;

	ntc_linear_start = table[boundary_start_index][0];
	ntc_linear_end = table[boundary_end_index][0];
	temp_linear_start = table[boundary_start_index][1];
	temp_linear_end = table[boundary_end_index][1];
	return ntc_linear_start +
		(cur_temp - temp_linear_start) *
		(ntc_linear_end - ntc_linear_start) /
		(temp_linear_end - temp_linear_start);
}

/*lint -e679*/
static int get_ntc_temperature(int resistance, int channel,
			       int **volt_to_temperature, int table_list)
{
	int low = 0;
	int upper = 0;
	int temperature = 0;
	int *temp_idex = (int *)volt_to_temperature;
	int mid;

	if (THERMAL_ADC2TEMP_FLAG == table_list)
		upper = sizeof(g_thermal_hkadc_volt_temp_table) /
			 sizeof(g_thermal_hkadc_volt_temp_table[0][0]) / 2;
	else if (THERMAL_PA_VOLT2TEMP_FLAG == table_list)
		upper = sizeof(g_thermal_pa_volt_temp_table) /
			 sizeof(g_thermal_pa_volt_temp_table[0][0]) / 2;
	else if (THERMAL_SOC_VOLT2TEMP_FLAG == table_list)
		upper = sizeof(g_thermal_soc_volt_temp_table) /
			 sizeof(g_thermal_soc_volt_temp_table[0][0]) / 2;

	if (resistance < *temp_idex) {
		temperature = *(temp_idex + 1) * PRORATE_OF_INIT;
		return temperature;
	} else if (resistance > *(temp_idex + 2 * (upper - 1))) {
		temperature = (*(temp_idex + 2 * (upper - 1) + 1)) * PRORATE_OF_INIT;
		return temperature;
	}

	while (low <= upper) {
		mid = (low + upper) / 2;
		if (*(temp_idex + 2 * mid) > resistance) {
			if (*(temp_idex + 2 * (mid - 1)) < resistance) {
				temperature =
					change_volt_to_temp(resistance,
							    mid - 1, mid,
							    table_list);
				return temperature;
			}
			upper = mid - 1;
		} else if (*(temp_idex + 2 * mid) < resistance) {
			if (*(temp_idex + 2 * (mid + 1)) > resistance) {
				temperature =
					change_volt_to_temp(resistance, mid,
							    mid + 1,
							    table_list);
				return temperature;
			}
			low = mid + 1;
		} else {
			temperature = (*(temp_idex + 2 * mid + 1)) * PRORATE_OF_INIT;
			break;
		}
	}
	return temperature;
}

/*lint +e679*/

/*lint -e573 -e679 -esym(573,679,*)*/
static int temp_2_ntc_bisearch_table(int cur_temp, int table[][2], int table_size)
{
	int upper = table_size / sizeof(int) / 2;
	int low = 0;
	int mid;

	if (cur_temp > table[0][1])
		return table[0][0];
	else if (cur_temp < table[upper - 1][1])
		return table[upper - 1][0];

	while (low <= upper) {
		mid = (low + upper) / 2;
		if (table[mid][1] > cur_temp) {
			if (table[mid + 1][1] < cur_temp)
				return temp_2_ntc_linear_value(cur_temp, mid,
							       mid + 1, table);

			low = mid + 1;
		} else if (table[mid][1] < cur_temp) {
			if (table[mid - 1][1] > cur_temp)
				return temp_2_ntc_linear_value(cur_temp, mid - 1,
							       mid, table);
			upper = mid - 1;
		} else {
			return table[mid][0];
		}
	}

	return table[0][0];
}

/*lint -e573 -e679 +esym(573,679,*)*/

int peripheral_ntc_2_temp(struct periph_tsens_tm_device_sensor *chip,
			       int *temp, int ntc)
{
	int tempdata = 0;

	*temp = tempdata;

	if (chip->ntc_name == NULL) {
		pr_err("ntc_name is NULL\n\r");
		return -EINVAL;
	}

	if (strncmp(chip->ntc_name, "austin_hkadc_adc_temp_table",
		    strlen(chip->ntc_name)) == 0)
		tempdata = get_ntc_temperature(ntc, chip->chanel,
					       (int **)g_thermal_hkadc_volt_temp_table,
					       THERMAL_ADC2TEMP_FLAG);
	else if (strncmp(chip->ntc_name, "austin_hkadc_pa_temp_table",
			 strlen(chip->ntc_name)) == 0)
		tempdata = get_ntc_temperature(ntc, chip->chanel,
					       (int **)g_thermal_pa_volt_temp_table,
					       THERMAL_PA_VOLT2TEMP_FLAG);
	else if (strncmp(chip->ntc_name, "austin_hkadc_soc_temp_table",
			 strlen(chip->ntc_name)) == 0)
		tempdata = get_ntc_temperature(ntc, chip->chanel,
					       (int **)g_thermal_soc_volt_temp_table,
					       THERMAL_SOC_VOLT2TEMP_FLAG);
	else
		pr_err("input ntc name was not found!\n\r");

	*temp = tempdata;
	return 0;
}

#ifdef CONFIG_HKADC_MODEM
int mdm_ntc_2_temp(struct hisi_mdm_adc_t *mdm_sensor, int *temp, int ntc)
{
	int tempdata = 0;

	*temp = tempdata;

	if (mdm_sensor->ntc_name == NULL) {
		pr_err("ntc_name is NULL\n\r");
		return -EINVAL;
	}

	if (strncmp(mdm_sensor->ntc_name, "austin_hkadc_adc_temp_table",
		    strlen(mdm_sensor->ntc_name)) == 0)
		tempdata = get_ntc_temperature(ntc, mdm_sensor->channel,
					       (int **)g_thermal_hkadc_volt_temp_table,
					       THERMAL_ADC2TEMP_FLAG);
	else if (strncmp(mdm_sensor->ntc_name, "austin_hkadc_pa_temp_table",
			 strlen(mdm_sensor->ntc_name)) == 0)
		tempdata = get_ntc_temperature(ntc, mdm_sensor->channel,
					       (int **)g_thermal_pa_volt_temp_table,
					       THERMAL_PA_VOLT2TEMP_FLAG);
	else if (strncmp(mdm_sensor->ntc_name, "austin_hkadc_soc_temp_table",
			 strlen(mdm_sensor->ntc_name)) == 0)
		tempdata = get_ntc_temperature(ntc, mdm_sensor->channel,
					       (int **)g_thermal_soc_volt_temp_table,
					       THERMAL_SOC_VOLT2TEMP_FLAG);
	else
		pr_err("input ntc name was not found!\n\r");

	*temp = tempdata;
	return 0;
}
#endif

int peripheral_get_table_info(const char *ntc_name, unsigned long *dest,
				   enum hkadc_table_id *table_id)
{
	if (ntc_name != NULL && table_id != NULL) {
		if (strncmp(ntc_name, "austin_hkadc_adc_temp_table",
			    strlen(ntc_name)) == 0) {
			*table_id = HKADC_ADC_TABLEID;
		} else if (strncmp(ntc_name, "austin_hkadc_pa_temp_table",
				   strlen(ntc_name)) == 0) {
			*table_id = HKADC_PA_TABLEID;
		} else if (strncmp(ntc_name, "austin_hkadc_soc_temp_table",
				   strlen(ntc_name)) == 0) {
			*table_id = HKADC_SOC_TABLEID;
		} else {
			*table_id = HKADC_MAX_TABLEID;
			pr_err("%s input ntc name was not found!\n", __func__);
		}

		return 0;
	}

	if (dest != NULL && table_id != NULL) {
		switch ((int)*table_id) {
		case HKADC_ADC_TABLEID:
			*dest = (unsigned long)(uintptr_t)g_thermal_hkadc_volt_temp_table;
			return sizeof(g_thermal_hkadc_volt_temp_table);
		case HKADC_PA_TABLEID:
			*dest = (unsigned long)(uintptr_t)g_thermal_pa_volt_temp_table;
			return sizeof(g_thermal_pa_volt_temp_table);
		/* SOC Table id, same as default */
		case HKADC_SOC_TABLEID:
		default:
			*dest = (unsigned long)(uintptr_t)g_thermal_soc_volt_temp_table;
			return sizeof(g_thermal_soc_volt_temp_table);
		}
	}

	return 0;
}

/* ntc:Maybe Resistance, Adc or Volt in usr module */
int peripheral_temp_2_ntc(struct periph_tsens_tm_device_sensor *chip,
			       int temp, u16 *ntc)
{
	if (chip->ntc_name == NULL) {
		pr_err("%s ntc_name is NULL\n", __func__);
		return -EINVAL;
	}

	if (strncmp(chip->ntc_name, "austin_hkadc_adc_temp_table",
		    strlen(chip->ntc_name)) == 0) {
		*ntc = (u16)temp_2_ntc_bisearch_table(temp,
						      g_thermal_hkadc_volt_temp_table,
						      sizeof(g_thermal_hkadc_volt_temp_table));
	} else if (strncmp(chip->ntc_name, "austin_hkadc_pa_temp_table",
			   strlen(chip->ntc_name)) == 0) {
		*ntc = (u16)temp_2_ntc_bisearch_table(temp,
						      g_thermal_pa_volt_temp_table,
						      sizeof(g_thermal_pa_volt_temp_table));
	} else if (strncmp(chip->ntc_name, "austin_hkadc_soc_temp_table",
			   strlen(chip->ntc_name)) == 0) {
		*ntc = (u16)temp_2_ntc_bisearch_table(temp,
						      g_thermal_soc_volt_temp_table,
						      sizeof(g_thermal_soc_volt_temp_table));
	} else {
		pr_err("input ntc name was not found!\n\r");
		return -EINVAL;
	}

	return 0;
}
