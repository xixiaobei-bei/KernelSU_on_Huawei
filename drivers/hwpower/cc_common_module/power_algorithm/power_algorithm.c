/*
 * power_algorithm.c
 *
 * algorithm (compensation \ hysteresis \ filter) interface for power module
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

#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_algo
HWLOG_REGIST();

#define POWER_COMP_ADC_CALC_RADIO   100000

unsigned char power_change_char_to_digit(unsigned char data)
{
	if (!isxdigit(data))
		return 0;

	if (isdigit(data))
		return data - '0';

	if (islower(data))
		return data - 'a' + POWER_BASE_DEC;

	if (isupper(data))
		return data - 'A' + POWER_BASE_DEC;

	return 0;
}

int power_get_min_value(const int *data, int size)
{
	int i;
	int min_value;

	if (!data)
		return 0;

	min_value = data[0];
	for (i = 0; i < size; ++i) {
		if (min_value > data[i])
			min_value = data[i];
	}

	return min_value;
}

int power_get_max_value(const int *data, int size)
{
	int i;
	int max_value;

	if (!data)
		return 0;

	max_value = data[0];
	for (i = 0; i < size; ++i) {
		if (max_value < data[i])
			max_value = data[i];
	}

	return max_value;
}

int power_get_average_value(const int *data, int size)
{
	int i;
	int sum_value = 0;

	if (!data || (size == 0))
		return 0;

	for (i = 0; i < size; ++i)
		sum_value += data[i];

	return sum_value / size;
}

/*
 * algorithm of common hysteresis
 * compare the current refer value with the refer value in the data table
 * and if it's going down happened, need the hysteresis value.
 */
int power_get_hysteresis_index(int index, const struct common_hys_data *data)
{
	int i, refer, refer_lth, hys_value;
	int new_index = index;

	if (!data)
		return index;

	refer = data->refer;
	refer_lth = data->para[index].refer_lth;
	hys_value = data->para[index].hys_value;

	for (i = 0; i < data->para_size; i++) {
		if ((refer >= data->para[i].refer_lth) &&
			(refer < data->para[i].refer_hth)) {
			/* down hysteresis */
			if ((index > i) && (refer_lth - refer > hys_value)) {
				new_index = i;
				break;
			} else if (index < i) {
				new_index = i;
				break;
			}
			break;
		}
	}

	hwlog_info("new_index:%d, refer:%d, lth:%d, hth:%d, hys:%d\n",
		new_index,
		data->refer,
		data->para[new_index].refer_lth,
		data->para[new_index].refer_hth,
		data->para[new_index].hys_value);
	return new_index;
}

/*
 * algorithm of common compensation
 * compare the current refer value with the refer value in the data table
 * to obtain the corresponding compensation value.
 */
int power_get_compensation_value(int raw, const struct common_comp_data *data)
{
	int i;
	int comp_value = raw;

	if (!data)
		return raw;

	for (i = 0; i < data->para_size; i++) {
		if (data->refer >= data->para[i].refer) {
			comp_value = raw - data->para[i].comp_value;
			break;
		}
	}

	hwlog_info("refer:%d, without_comp:%d, with_comp:%d\n",
		data->refer, raw, comp_value);
	return comp_value;
}

/*
 * algorithm of common smooth compensation
 * compare the current value with the last value in the data table
 * to obtain the smooth compensation value.
 */
int power_get_smooth_compensation_value(const struct smooth_comp_data *data)
{
	int current_comp, current_raw, delta_comp, delta_raw;

	current_comp = data->current_comp;
	current_raw = data->current_raw;
	delta_comp = current_comp - data->last_comp;
	delta_raw = current_raw - data->last_raw;
	if ((delta_comp < 0) && (delta_raw > 0))
		current_comp = data->last_comp;
	else if ((delta_comp > 0) && (delta_raw < 0))
		current_comp = data->last_comp;
	else if (abs(delta_comp) > abs(delta_raw))
		current_comp = data->last_comp + delta_raw;

	if (current_comp - data->last_comp > data->max_delta)
		current_comp = data->last_comp + data->max_delta;
	else if (data->last_comp - current_comp > data->max_delta)
		current_comp = data->last_comp - data->max_delta;

	hwlog_info("current_comp:%d, c_comp:%d, c_raw:%d, l_comp:%d, l_raw:%d\n",
		current_comp,
		data->current_comp, data->current_raw,
		data->last_comp, data->last_raw);
	return current_comp;
}

/*
 * algorithm of ground circuit compensation
 */
int power_get_adc_compensation_value(int adc_value, const struct adc_comp_data *data)
{
	s64 tmp;
	int v_adc;
	int r_ntc;

	if (!data || !data->adc_accuracy || !data->r_pullup)
		return -1;

	/* only support adc accuracy less than 16 bit */
	/* adc_v_ref / adc_accuracy = v_adc / adc_value */
	tmp = (s64)(data->adc_v_ref) * (s64)adc_value * POWER_COMP_ADC_CALC_RADIO;
	tmp = div_s64(tmp, BIT(data->adc_accuracy));
	v_adc = div_s64(tmp, POWER_COMP_ADC_CALC_RADIO);
	if (data->v_pullup - v_adc == 0)
		return -1;

	/* v_adc / (r_ntc + r_comp) = (v_pullup - v_adc) / r_pullup */
	r_ntc = v_adc * data->r_pullup / (data->v_pullup - v_adc) - data->r_comp;
	hwlog_info("v_adc:%d, r_pullup:%d, v_pullup:%d, r_comp:%d, r_ntc:%d\n",
		v_adc, data->r_pullup, data->v_pullup, data->r_comp, r_ntc);

	return r_ntc;
}

int power_convert_value(const struct convert_data *data, int len, int refer, int *new_val)
{
	int i;

	if (!data || !new_val)
		return -EPERM;

	for (i = 0; i < len; i++) {
		if (refer != data[i].refer)
			continue;
		*new_val = data[i].new_val;
		hwlog_info("refer:%d, after conversion val:%d\n", refer, *new_val);
		return 0;
	}

	hwlog_err("refer %d is illegal\n", refer);
	return -1;
}
