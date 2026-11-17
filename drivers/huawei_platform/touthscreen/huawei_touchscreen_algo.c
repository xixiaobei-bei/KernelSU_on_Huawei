/*
 * Huawei Touchscreen Driver
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
#include <huawei_platform/touthscreen/huawei_touchscreen.h>
#include <huawei_platform/touthscreen/huawei_ts_kit_misc_dev.h>

int ts_algo_t1(struct ts_device_data *dev_data, struct ts_fingers *in_info,
	struct ts_fingers *out_info)
{
	int index;
	int id;

	if (dev_data == NULL)
		ts_log_debug("%s get chip data NULL\n", __func__);

	memset(out_info, 0, sizeof(struct ts_fingers));
	for (index = 0, id = 0; index < TS_MAX_FINGER; index++, id++) {
		if (in_info->cur_finger_number == 0) {
			out_info->fingers[0].status = TS_FINGER_RELEASE;
			if (id >= 1)
				out_info->fingers[id].status = 0;
		} else {
			if (((in_info->fingers[index].x != 0) ||
				(in_info->fingers[index].y != 0))) {
				out_info->fingers[id].x =
					in_info->fingers[index].x;
				out_info->fingers[id].y =
					in_info->fingers[index].y;
				out_info->fingers[id].pressure =
					in_info->fingers[index].pressure;
				out_info->fingers[id].status = TS_FINGER_PRESS;
			} else {
				out_info->fingers[id].status = 0;
			}
		}
	}
	out_info->cur_finger_number = in_info->cur_finger_number;
	out_info->gesture_wakeup_value = in_info->gesture_wakeup_value;
	out_info->special_button_key = in_info->special_button_key;
	out_info->special_button_flag = in_info->special_button_flag;
	return NO_ERR;
}


struct ts_algo_func ts_algo_f1 = {
	.algo_name = "ts_algo_f1",
	.chip_algo_func = ts_algo_t1,
};

struct ts_algo_func ts_algo_f2 = {
	.algo_name = "ts_algo_f2",
	.chip_algo_func = ts_algo_t1,
};

int ts_register_algo_func(struct ts_device_data *chip_data)
{
	int retval;

	retval = register_algo_func(chip_data, &ts_algo_f1);
	if (retval < 0) {
		ts_log_err("alog 1 failed, retval = %d\n", retval);
		return retval;
	}

	retval = register_algo_func(chip_data, &ts_algo_f2);
	if (retval < 0) {
		ts_log_err("alog 2 failed, retval = %d\n", retval);
		return retval;
	}

	return retval;
}
