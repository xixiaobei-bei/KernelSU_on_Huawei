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
 *
 */

#ifndef __HUAWEI_TS_KIT_MISC_H_
#define __HUAWEI_TS_KIT_MISC_H_

#include <linux/ioctl.h>
#include <linux/miscdevice.h>

#define DEVICE_AFT_GET_INFO "ts_aft_get_info"
#define DEVICE_AFT_SET_INFO "ts_aft_set_info"
/* commands */
#define INPUT_AFT_GET_IO_TYPE 0xBA
#define INPUT_AFT_SET_IO_TYPE 0xBB

#define STATUS_FLAG_INIT_VALUE 0
#define STYLUS3_RECEIVE_ACK 1
#define STYLUS3_CLEAR_ACK 0
#define EASYWAKE_POSITION_SHIFT 16

struct double_click_data {
	unsigned int key_code;
	unsigned int x_position;
	unsigned int y_position;
};

#define INPUT_AFT_IOCTL_CMD_GET_TS_FINGERS_INFO \
	_IOWR(INPUT_AFT_GET_IO_TYPE, 0x01, struct ts_normal_fingers)
#define INPUT_AFT_IOCTL_CMD_GET_FOLD_TS_FINGERS_INFO \
	_IOWR(INPUT_AFT_GET_IO_TYPE, 0x01, struct ts_fingers)
#define INPUT_AFT_IOCTL_CMD_GET_ALGO_PARAM_INFO \
	_IOWR(INPUT_AFT_GET_IO_TYPE, 0x02, struct ts_aft_algo_param)
#define INPUT_AFT_IOCTL_CMD_GET_DIFF_DATA_INFO \
	_IOWR(INPUT_AFT_GET_IO_TYPE, 0x03, char)
#define INPUT_AFT_IOCTL_CMD_GET_TS_FINGER_PEN_INFO \
	_IOWR(INPUT_AFT_GET_IO_TYPE, 0x04, struct ts_finger_pen_info)
#define INPUT_AFT_IOCTL_CMD_GET_STYLUS3_CONNECT_STATUS \
	_IOWR(INPUT_AFT_GET_IO_TYPE, 0x05, unsigned int)
#define INPUT_AFT_IOCTL_CMD_GET_CALLBACK_EVENTS \
	_IOWR(INPUT_AFT_GET_IO_TYPE, 0x07, struct tp_callback_event)
#define IOCTL_CMD_READ_STYLUS3_SHIFT_FREQ_INFO \
	_IOWR(INPUT_AFT_GET_IO_TYPE, 0x06, struct shift_freq_info)
#define INPUT_AFT_IOCTL_CMD_GET_BTPEN_TYPE_TO_TPIC \
	_IOWR(INPUT_AFT_GET_IO_TYPE, 0x08, u32)
#define INPUT_AFT_IOCTL_CMD_SET_COORDINATES \
	_IOWR(INPUT_AFT_SET_IO_TYPE, 0x01, struct ts_normal_fingers)
#define INPUT_AFT_IOCTL_CMD_SET_FOLD_COORDINATES \
	_IOWR(INPUT_AFT_SET_IO_TYPE, 0x01, struct ts_fingers)
#define INPUT_AFT_IOCTL_CMD_SET_SENSIBILITY_CFG \
	_IOWR(INPUT_AFT_SET_IO_TYPE, 0x02, int)
#define INPUT_IOCTL_CMD_SET_FLIP_KEY \
	_IOWR(INPUT_AFT_SET_IO_TYPE, 0x03, unsigned int)
#define INPUT_IOCTL_CMD_SET_DOUBLE_CLICK \
	_IOWR(INPUT_AFT_SET_IO_TYPE, 0x04, struct double_click_data)
#define INPUT_IOCTL_CMD_SET_TP_IC_COMMAND \
	_IOWR(INPUT_AFT_SET_IO_TYPE, 0x05, struct tp_ic_command)
#define INPUT_AFT_IOCTL_CMD_SET_PENS \
	_IOWR(INPUT_AFT_SET_IO_TYPE, 0x07, struct ts_pens)
#define INPUT_AFT_IOCTRL_CMD_SET_STYLUS3_CONNECT_STATUS \
	_IOWR(INPUT_AFT_SET_IO_TYPE, 0x08, unsigned int)
#define INPUT_AFT_IOCTL_CMD_SET_CALLBACK_EVENTS \
	_IOWR(INPUT_AFT_SET_IO_TYPE, 0x09, struct tp_callback_event)
#define INPUT_AFT_IOCTL_CMD_SET_STYLUS3_ACK_FREQ \
	_IOWR(INPUT_AFT_SET_IO_TYPE, 0x0A, struct stylus3_ack_freq)
#define INPUT_AFT_IOCTL_CMD_SET_BTPEN_TYPE_TO_TPIC \
	_IOWR(INPUT_AFT_SET_IO_TYPE, 0x0b, u32)
#define INPUT_AFT_IOCTL_CMD_SET_STYLUS3_PLAM_SUPPRESSION_STATUS \
	_IOWR(INPUT_AFT_SET_IO_TYPE, 0x0c, u32)

int copy_fingers_to_aft_daemon(struct ts_kit_platform_data *pdata,
	struct ts_fingers *fingers);
int copy_finger_pen_to_aft_daemon(struct ts_kit_platform_data *pdata,
	struct ts_finger_pen_info *finger_pen);
void copy_stylus3_shift_freq_to_aft_daemon(
	struct shift_freq_info *shif_freq_info);
void ts_update_stylu3_shif_freq_flag(unsigned int status, bool set_flag);
int ts_kit_misc_init(struct ts_kit_platform_data *pdata);
void ts_kit_misc_destroy(void);

extern struct ts_kit_platform_data g_ts_kit_platform_data;
extern atomic_t g_ts_kit_data_report_over;

#endif
