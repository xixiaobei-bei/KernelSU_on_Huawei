/*
 * Huawei Touchscreen Driver
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
 */

#include "huawei_ts_kit_algo.h"
#include <linux/ktime.h>

#define EDGE_Y_MID 1380
#define EDGE_Y_MAX 1919
#define EDGE_X_MAX 1079

#define STOP_LIMITED 150

#define Y_MOVE_S 20
#define Y_MOVE_M 40
#define Y_TEMP_W 40
#define Y_START_W 60

#define CENTER_X 500
#define OFFSET_DIS 150
#define RES_ADDR 0
#define RES_ERT 1
#define RES_ERF 2
#define RES_CRF 3
#define RES_ERT_THR 2
#define SET_TO_NEGATIVE_NUM (-1)
#define SENCOND_TO_MILLISECOND 1000
#define SENCOND_TO_NANOSECOND 1000000

#define TEMP_EDGE 20
#define START_EDGE 30
#define START_STUDY_EDGE 40

#define LEFT_RES_POINT_SIZE 4
#define RIGHT_RES_POINT_SIZE 4
#define RES_ADDR_OFFSET_2BIT 2
#define RES_ADDR_MULTI 3
#define FINGER_STOP_FLAG 1

static int stop_left;
static int stop_right;
static int temp_left;
static int temp_right;
static int start_left;
static int start_right;
static int start_study_left;
static int start_study_right;

static int temp_up;
static int temp_down;
static int start_up;
static int start_down;

struct ts_kit_platform_data __attribute__((weak)) g_ts_kit_platform_data;
static struct timespec curr_time[FILTER_GLOVE_NUMBER] = { { 0, 0 } };
static struct timespec pre_finger_time[FILTER_GLOVE_NUMBER] = { { 0, 0 } };

static int touch_pos_x[FILTER_GLOVE_NUMBER] = { -1, -1, -1, -1 };
static int touch_pos_y[FILTER_GLOVE_NUMBER] = { -1, -1, -1, -1 };
static enum TP_state_machine touch_state = INIT_STATE;

static u16 must_report_flag;
static u16 pre_must_flag;
static u16 stop_report_flag;
static u16 temp_report_flag;
static int finger_stop_cnt[TS_MAX_FINGER] = {0};
static int finger_stop_y[TS_MAX_FINGER] = {0};

/* 0 point addr, 1 error times, 2 error flag, 3 correct flag */
static int left_res_point[LEFT_RES_POINT_SIZE] = {0};
static int right_res_point[RIGHT_RES_POINT_SIZE] = {0};

static int filter_illegal_glove(u8 n_finger, struct ts_fingers *in_info)
{
	u8 report_flag = 0;
	long interval_time;
	u8 new_mode;
	int x = in_info->fingers[n_finger].x;
	int y = in_info->fingers[n_finger].y;

	new_mode = in_info->fingers[n_finger].status;
	/* the new interrupt is a finger signal */
	if ((new_mode == TP_FINGER) ||
		g_ts_kit_platform_data.feature_info.holster_info.holster_switch) {
		touch_state = FINGER_STATE;
		report_flag = 1; /* 0 or 1 is to judge glove mode legality */
	/* the new interrupt is a glove signal */
	} else if ((new_mode == TP_GLOVE) || (new_mode == TP_STYLUS)) {
		switch (touch_state) {
		case INIT_STATE:
			report_flag = 1;
			touch_state = GLOVE_STATE;
			break;
		case FINGER_STATE:
			ktime_get_ts(&curr_time[n_finger]);
			interval_time = ((curr_time[n_finger].tv_sec -
				pre_finger_time[n_finger].tv_sec) *
				SENCOND_TO_MILLISECOND) +
				((curr_time[n_finger].tv_nsec -
				pre_finger_time[n_finger].tv_nsec) /
				SENCOND_TO_NANOSECOND);
			if ((interval_time > 0) &&
				(interval_time <= FINGER_REL_TIME))
				ktime_get_ts(&pre_finger_time[n_finger]);
			else
				touch_state = ZERO_STATE;
			break;
		case ZERO_STATE:
			if ((touch_pos_x[n_finger] == -1) &&
				(touch_pos_y[n_finger] == -1)) {
				touch_pos_x[n_finger] = x;
				touch_pos_y[n_finger] = y;
			} else {
				if (((touch_pos_x[n_finger] - x) *
					(touch_pos_x[n_finger] - x) +
					(touch_pos_y[n_finger] - y) *
					(touch_pos_y[n_finger] - y)) >=
					(PEN_MOV_LENGTH * PEN_MOV_LENGTH))
					touch_state = GLOVE_STATE;
			}
			break;
		case GLOVE_STATE:
			report_flag = 1;
			break;
		default:
			TS_LOG_ERR("error: touch_state = %d\n", touch_state);
			break;
		}
	} else {
		TS_LOG_ERR("%s: cur_mode=%d\n", __func__, new_mode);
		report_flag = 1;
	}

	return report_flag;
}

int ts_kit_algo_t2(struct ts_kit_device_data *dev_data,
	struct ts_fingers *in_info, struct ts_fingers *out_info)
{
	int index;
	unsigned int id;
	struct anti_false_touch_param *local_param = NULL;

	if (!in_info || !out_info) {
		TS_LOG_ERR("%s: find a null pointer\n", __func__);
		return -EINVAL;
	}
	if (dev_data == NULL) {
		TS_LOG_DEBUG("%s anti false touch get chip data NULL\n",
			__func__);
		local_param = NULL;
	} else {
		local_param = &(dev_data->anti_false_touch_param_data);
	}

	memset(out_info, 0, sizeof(*out_info));

	for (index = 0, id = 0; index < TS_MAX_FINGER; index++, id++) {
		if (in_info->cur_finger_number == 0) {
			out_info->fingers[0].status = TS_FINGER_RELEASE;
			if (local_param && local_param->feature_all)
				local_param->edge_status = 0;
			if (id >= 1)
				out_info->fingers[id].status = 0;
		} else {
			if ((in_info->fingers[index].x != 0) ||
				(in_info->fingers[index].y != 0)) {
				out_info->fingers[id].x =
					in_info->fingers[index].x;
				out_info->fingers[id].y =
					in_info->fingers[index].y;
				out_info->fingers[id].pressure =
					in_info->fingers[index].pressure;
				out_info->fingers[id].major =
					in_info->fingers[index].major;
				out_info->fingers[id].minor =
					in_info->fingers[index].minor;
				out_info->fingers[id].wx =
					in_info->fingers[index].wx;
				out_info->fingers[id].wy =
					in_info->fingers[index].wy;
				out_info->fingers[id].ewx =
					in_info->fingers[index].ewx;
				out_info->fingers[id].ewy =
					in_info->fingers[index].ewy;
				out_info->fingers[id].xer =
					in_info->fingers[index].xer;
				out_info->fingers[id].yer =
					in_info->fingers[index].yer;
				out_info->fingers[id].orientation =
					in_info->fingers[index].orientation;
				out_info->fingers[id].status = TS_FINGER_PRESS;
			} else {
				out_info->fingers[id].status = 0;
				if (local_param && local_param->feature_all)
					local_param->edge_status &= ~(1 << id);
			}
		}
	}
	out_info->cur_finger_number = in_info->cur_finger_number;
	out_info->gesture_wakeup_value = in_info->gesture_wakeup_value;
	out_info->special_button_key = in_info->special_button_key;
	out_info->special_button_flag = in_info->special_button_flag;

	return NO_ERR;
}

int ts_kit_algo_t1(struct ts_kit_device_data *dev_data,
	struct ts_fingers *in_info, struct ts_fingers *out_info)
{
	int index;
	unsigned int id;
	int finger_cnt = 0;
	struct anti_false_touch_param *local_param = NULL;

	if (!in_info || !out_info) {
		TS_LOG_ERR("%s : find a null pointer\n", __func__);
		return -EINVAL;
	}
	if (dev_data == NULL) {
		TS_LOG_ERR("%s anti false touch get chip data NULL\n",
			__func__);
		local_param = NULL;
	} else {
		local_param = &(dev_data->anti_false_touch_param_data);
	}

	memset(out_info, 0, sizeof(*out_info));

	for (index = 0, id = 0; index < TS_MAX_FINGER; index++, id++) {
		if (in_info->cur_finger_number == 0) {
			if (index < FILTER_GLOVE_NUMBER) {
				touch_pos_x[index] = -1;
				touch_pos_y[index] = -1;
				/* this is a finger release */
				if (touch_state == FINGER_STATE)
					ktime_get_ts(&pre_finger_time[index]);
			}
			out_info->fingers[0].status = TS_FINGER_RELEASE;
			if (local_param && local_param->feature_all)
				local_param->edge_status = 0;
			if (id >= 1)
				out_info->fingers[id].status = 0;
		} else {
			if ((in_info->fingers[index].x != 0) ||
				(in_info->fingers[index].y != 0)) {
				if (index < FILTER_GLOVE_NUMBER) {
					if (filter_illegal_glove(index, in_info) == 0) {
						out_info->fingers[id].status = 0;
					} else {
						finger_cnt++;
						out_info->fingers[id].x =
							in_info->fingers[index].x;
						out_info->fingers[id].y =
							in_info->fingers[index].y;
						out_info->fingers[id].pressure =
							in_info->fingers[index].pressure;
						out_info->fingers[id].major =
							in_info->fingers[index].major;
						out_info->fingers[id].minor =
							in_info->fingers[index].minor;
						out_info->fingers[id].wx =
							in_info->fingers[index].wx;
						out_info->fingers[id].wy =
							in_info->fingers[index].wy;
						out_info->fingers[id].ewx =
							in_info->fingers[index].ewx;
						out_info->fingers[id].ewy =
							in_info->fingers[index].ewy;
						out_info->fingers[id].xer =
							in_info->fingers[index].xer;
						out_info->fingers[id].yer =
							in_info->fingers[index].yer;
						out_info->fingers[id].orientation =
							in_info->fingers[index].orientation;
						out_info->fingers[id].status =
							TS_FINGER_PRESS;
					}
				} else {
					finger_cnt++;
					out_info->fingers[id].x =
						in_info->fingers[index].x;
					out_info->fingers[id].y =
						in_info->fingers[index].y;
					out_info->fingers[id].pressure =
						in_info->fingers[index].pressure;
					out_info->fingers[id].major =
						in_info->fingers[index].major;
					out_info->fingers[id].minor =
						in_info->fingers[index].minor;
					out_info->fingers[id].wx =
						in_info->fingers[index].wx;
					out_info->fingers[id].wy =
						in_info->fingers[index].wy;
					out_info->fingers[id].ewx =
						in_info->fingers[index].ewx;
					out_info->fingers[id].ewy =
						in_info->fingers[index].ewy;
					out_info->fingers[id].xer =
						in_info->fingers[index].xer;
					out_info->fingers[id].yer =
						in_info->fingers[index].yer;
					out_info->fingers[id].orientation =
						in_info->fingers[index].orientation;
					out_info->fingers[id].status =
						TS_FINGER_PRESS;
				}
			} else {
				out_info->fingers[id].status = 0;
				if (local_param && local_param->feature_all)
					local_param->edge_status &= ~(1 << id);
			}
		}
	}
	out_info->cur_finger_number = finger_cnt;
	out_info->gesture_wakeup_value = in_info->gesture_wakeup_value;
	out_info->special_button_key = in_info->special_button_key;
	out_info->special_button_flag = in_info->special_button_flag;

	return NO_ERR;
}

static int stop_to_start(int x, int y, int start_y, int cnt, int *point)
{
	int temp_value;

	temp_value = y - start_y;
	if (!((y > (point[RES_ADDR] - OFFSET_DIS)) &&
		(y < (point[RES_ADDR] + OFFSET_DIS)))) {
		if (temp_value > Y_MOVE_S)
			return FINGER_STOP_FLAG;
		if (temp_value < 0) {
			temp_value = 0 - temp_value;
			if (temp_value > Y_MOVE_M)
				return FINGER_STOP_FLAG;
		}
		if ((x > start_left) && (x < start_right) &&
			(y > start_up && y < start_down))
			return FINGER_STOP_FLAG;
	} else {
		if ((x > (start_study_left + cnt)) &&
			(x < (start_study_right - cnt)) &&
			(y > start_up && y < start_down))
			return FINGER_STOP_FLAG;
	}
	return 0;
}

static int start_stop_area(int x, int y, int *point)
{
	if (x < stop_left || x > stop_right)
		return 1; /* finger out of area flag */
	return 0;
}

static int update_restrain_area(int y, int *point)
{
	/* restrain area at the bottom screen */
	if (y > EDGE_Y_MID) {
		/* restrain area not exist, set it up */
		if (point[RES_ADDR] == 0) {
			point[RES_CRF] = 1; /* correct flag */
			point[RES_ADDR] = y;
		} else if ((y > (point[RES_ADDR] - OFFSET_DIS)) &&
			(y < (point[RES_ADDR] + OFFSET_DIS))) { /* adjust area */
			point[RES_CRF] = 1; /* correct flag */
			point[RES_ADDR] =
				(point[RES_ADDR] * RES_ADDR_MULTI + y) >>
				RES_ADDR_OFFSET_2BIT;
			TS_LOG_DEBUG("restrain point updated: %d\n",
				point[RES_ADDR]);
		} else { /* not in the current restrain area */
			point[RES_ERF] = 1; /* error flag */
		}
	}
	return 0;
}

int ts_kit_algo_t3(struct ts_kit_device_data *dev_data,
	struct ts_fingers *in_info, struct ts_fingers *out_info)
{
	unsigned int index;
	int temp_x;
	int temp_y;
	int temp_val;
	int *temp_point = NULL;

	if (!in_info || !out_info) {
		TS_LOG_ERR("%s : find a null pointer\n", __func__);
		return -EINVAL;
	}
	if (g_ts_kit_platform_data.edge_wideth == 0)
		return NO_ERR;
	if (g_ts_kit_platform_data.edge_wideth != stop_left) {
		stop_left = g_ts_kit_platform_data.edge_wideth;
		stop_right = EDGE_X_MAX - stop_left;
		temp_left = stop_left + TEMP_EDGE;
		temp_right = stop_right - TEMP_EDGE;

		start_left = stop_left + START_EDGE;
		start_right = stop_right - START_EDGE;

		temp_up = Y_TEMP_W;
		temp_down = EDGE_Y_MAX - Y_TEMP_W;
		start_up = Y_START_W;
		start_down = EDGE_Y_MAX - Y_START_W;

		start_study_left = stop_left + START_STUDY_EDGE;
		start_study_right = stop_right - START_STUDY_EDGE;
	}

	if (in_info->cur_finger_number == 0) {
		TS_LOG_DEBUG("no finger, only a release issue\n");

		if ((left_res_point[RES_CRF] == 0) &&
			(left_res_point[RES_ERF] != 0)) {
			if (left_res_point[RES_ERT] < RES_ERT_THR) {
				left_res_point[RES_ERT]++;
				if (left_res_point[RES_ERT] >= RES_ERT_THR) {
					left_res_point[RES_ADDR] = 0;
					left_res_point[RES_ERT] = 0;
				}
			}
		}
		if ((right_res_point[RES_CRF] == 0) &&
			(right_res_point[RES_ERF] != 0)) {
			if (right_res_point[RES_ERT] < RES_ERT_THR) {
				right_res_point[RES_ERT]++;
				if (right_res_point[RES_ERT] >= RES_ERT_THR) {
					right_res_point[RES_ADDR] = 0;
					right_res_point[RES_ERT] = 0;
				}
			}
		}
		left_res_point[RES_CRF] = 0;
		left_res_point[RES_ERF] = 0;
		right_res_point[RES_CRF] = 0;
		right_res_point[RES_ERF] = 0;

		must_report_flag = 0;
		stop_report_flag = 0;
		temp_report_flag = 0;
		for (index = 0; index < TS_MAX_FINGER; index++) {
			finger_stop_cnt[index] = 0;
			finger_stop_y[index] = 0;
		}
	} else {
		for (index = 0; index < TS_MAX_FINGER; index++) {
			if (in_info->fingers[index].status != 0) {
				temp_x = in_info->fingers[index].x;
				temp_y = in_info->fingers[index].y;
				if (must_report_flag & (1 << index)) {
					TS_LOG_DEBUG("finger index: %u, is reporting\n", index);

					out_info->fingers[index].x = temp_x;
					out_info->fingers[index].y = temp_y;
					out_info->fingers[index].pressure = in_info->fingers[index].pressure;
					out_info->fingers[index].status = TS_FINGER_PRESS;
				} else {
					temp_point = left_res_point;
					if (temp_x > CENTER_X)
						temp_point = right_res_point;
					if (stop_report_flag & (1 << index)) {
						temp_val = stop_to_start(temp_x, temp_y, finger_stop_y[index],
							finger_stop_cnt[index], temp_point);
						TS_LOG_DEBUG("stop_to_start ret_value: %d,\n", temp_val);
						if (temp_val) {
							TS_LOG_DEBUG(
								"stopped finger index: %u, coordinate is legal1, can report again\n",
								index);

							must_report_flag |= (1 << index);
							stop_report_flag &= ~(1 << index);

							out_info->fingers[index].x = temp_x;
							out_info->fingers[index].y = temp_y;
							out_info->fingers[index].pressure =
								in_info->fingers[index].pressure;
							out_info->fingers[index].status = TS_FINGER_PRESS;
						} else {
							out_info->fingers[index].status = 0;
							if (finger_stop_cnt[index] < STOP_LIMITED)
								finger_stop_cnt[index]++;
							TS_LOG_DEBUG(
								"finger index: %u, keep stopped, stop_cnt: %d\n",
								index, finger_stop_cnt[index]);
							update_restrain_area(temp_y, temp_point);
						}
					} else {
						/* this area maybe need to restrain */
						if ((temp_x < temp_left) || (temp_x > temp_right) ||
							(temp_y < temp_up) || (temp_y > temp_down)) {
							TS_LOG_DEBUG("finger index: %u, stop judge\n", index);

							/* don't report first (important) */
							out_info->fingers[index].status = 0;

							/* has been report first */
							if (!(temp_report_flag & (1 << index))) {
								/* current point in direct restrain area */
								if (start_stop_area(temp_x, temp_y, temp_point)) {
									TS_LOG_DEBUG("finger need stopped\n");

									stop_report_flag |= (1 << index);
									finger_stop_cnt[index] = 1;
									finger_stop_y[index] = temp_y;
									update_restrain_area(temp_y, temp_point);
								} else {
									temp_report_flag |= (1 << index);
								}
							}
						} else {
							TS_LOG_DEBUG(
								"finger index: %u, start OK, directly report\n",
								index);

							/* not in edge area forever */
							if ((!(temp_report_flag & (1 << index))) ||
								(((temp_x > start_left) && (temp_x < start_right)) &&
								(temp_y > start_up && temp_y < start_down))) {
								/* not in edge area now */
								must_report_flag |= (1 << index);
								temp_report_flag &= ~(1 << index);
							}

							out_info->fingers[index].x = in_info->fingers[index].x;
							out_info->fingers[index].y = in_info->fingers[index].y;
							out_info->fingers[index].pressure =
								in_info->fingers[index].pressure;
							out_info->fingers[index].status = TS_FINGER_PRESS;
						}
					}
				}
			} else {
				out_info->fingers[index].status = 0;
				must_report_flag &= ~(1 << index);
				stop_report_flag &= ~(1 << index);
				temp_report_flag &= ~(1 << index);
				finger_stop_cnt[index] = 0;
				finger_stop_y[index] = 0;
			}
		}

		TS_LOG_DEBUG(
			"1_must_report_flag=%d, stop=%d, temp=%d, L_RES_ADDR=%d, L_RES_ERT=%d, R_RES_ADDR=%d, R_RES_ERT=%d\n",
			must_report_flag, stop_report_flag, temp_report_flag,
			left_res_point[RES_ADDR], left_res_point[RES_ERT],
			right_res_point[RES_ADDR], right_res_point[RES_ERT]);

		if (temp_report_flag) {
			if (must_report_flag) {
				for (index = 0; index < TS_MAX_FINGER; index++) {
					if (temp_report_flag & (1 << index)) {
						if (!(stop_report_flag & (1 << index))) {
							finger_stop_cnt[index] = 1;
							finger_stop_y[index] = in_info->fingers[index].y;
							out_info->fingers[index].status = 0;
						}
					}
				}
				stop_report_flag |= temp_report_flag;
				temp_report_flag = 0;

				if (pre_must_flag == 0)
					out_info->fingers[0].status = TS_FINGER_RELEASE;
			} else {
				for (index = 0; index < TS_MAX_FINGER; index++) {
					/*
					 * temp_report_flag bit won't
					 * the same with stop_report_flag bit
					 */
					if (temp_report_flag & (1 << index)) {
						out_info->fingers[index].x =
							in_info->fingers[index].x;
						out_info->fingers[index].y =
							in_info->fingers[index].y;
						out_info->fingers[index].pressure =
							in_info->fingers[index].pressure;
						out_info->fingers[index].status =
							TS_FINGER_PRESS;
					}
				}
			}
		}

		TS_LOG_DEBUG(
			"2_must_report_flag=%d, stop=%d, temp=%d, L_RES_ADDR=%d, L_RES_ERT=%d, R_RES_ADDR=%d, R_RES_ERT=%d\n",
			must_report_flag, stop_report_flag, temp_report_flag,
			left_res_point[RES_ADDR], left_res_point[RES_ERT],
			right_res_point[RES_ADDR], right_res_point[RES_ERT]);

		if ((must_report_flag | temp_report_flag) == 0)
			out_info->fingers[0].status = TS_FINGER_RELEASE;
	}
	pre_must_flag = must_report_flag;
	return NO_ERR;
}

struct ts_algo_func ts_kit_algo_f1 = {
	.algo_name = "ts_kit_algo_f1",
	.chip_algo_func = ts_kit_algo_t1,
};

struct ts_algo_func ts_kit_algo_f2 = {
	.algo_name = "ts_kit_algo_f2",
	.chip_algo_func = ts_kit_algo_t2,
};

struct ts_algo_func ts_kit_algo_f3 = {
	.algo_name = "ts_kit_algo_f3",
	.chip_algo_func = ts_kit_algo_t3,
};

int register_ts_algo_func(struct ts_kit_device_data *chip_data,
	struct ts_algo_func *fn)
{
	int error;

	if (!chip_data || !fn) {
		error = -EIO;
		goto out;
	}

	fn->algo_index = chip_data->algo_size;
	list_add_tail(&fn->node, &chip_data->algo_head);
	chip_data->algo_size++;
	/* We use barrier to make sure data consistently */
	smp_mb();
	error = NO_ERR;

out:
	return error;
}
EXPORT_SYMBOL(register_ts_algo_func);

int ts_kit_register_algo_func(struct ts_kit_device_data *chip_data)
{
	int retval;

	/* put algo_f1 into list contained in chip_data, named algo_t1 */
	retval = register_ts_algo_func(chip_data, &ts_kit_algo_f1);
	if (retval < 0) {
		TS_LOG_ERR("alog 1 failed, retval = %d\n", retval);
		return retval;
	}

	/* put algo_f2 into list contained in chip_data, named algo_t2 */
	retval = register_ts_algo_func(chip_data, &ts_kit_algo_f2);
	if (retval < 0) {
		TS_LOG_ERR("alog 2 failed, retval = %d\n", retval);
		return retval;
	}

	/* put algo_f3 into list contained in chip_data, named algo_t3 */
	retval = register_ts_algo_func(chip_data, &ts_kit_algo_f3);
	if (retval < 0) {
		TS_LOG_ERR("alog 3 failed, retval = %d\n", retval);
		return retval;
	}

	return retval;
}
