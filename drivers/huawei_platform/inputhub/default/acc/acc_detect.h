/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: acc detect header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ACC_DETECT_H__
#define __ACC_DETECT_H__

#include "sensor_sysfs.h"

#define ACC_DEV_COUNT_MAX 2

#define ACC_OFFSET_CALIBRATE_LENGTH 3

struct g_sensor_platform_data {
	struct sensor_combo_cfg cfg;
	uint8_t axis_map_x;
	uint8_t axis_map_y;
	uint8_t axis_map_z;
	uint8_t negate_x;
	uint8_t negate_y;
	uint8_t negate_z;
	uint8_t used_int_pin;
	GPIO_NUM_TYPE gpio_int1;
	GPIO_NUM_TYPE gpio_int2;
	GPIO_NUM_TYPE gpio_int2_sh;
	uint16_t poll_interval;
	int offset_x;
	int offset_y;
	int offset_z;
	int sensitivity_x;
	int sensitivity_y;
	int sensitivity_z;
	uint8_t device_type;
	uint8_t calibrate_style;
	uint8_t calibrate_way;
	uint16_t x_calibrate_thredhold;
	uint16_t y_calibrate_thredhold;
	uint16_t z_calibrate_thredhold;
	uint8_t wakeup_duration;
	uint8_t g_sensor_extend_data[SENSOR_PLATFORM_EXTEND_DATA_SIZE];
	uint8_t gpio_int2_sh_func;
	uint8_t acc_data_thredhold; /* uint: m/s2 */
};

struct acc_device_info {
	int32_t obj_tag;
	uint32_t detect_list_id;
	uint8_t acc_dev_index;
	uint8_t acc_first_start_flag;
	uint8_t acc_opened_before_calibrate;
	enum ret_type acc_calibration_res;
};

struct g_sensor_platform_data *acc_get_platform_data(int32_t tag);
struct acc_device_info *acc_get_device_info(int32_t tag);
char *acc_get_sensors_id_string(void);
void acc_get_sensors_id_from_dts(struct device_node *dn);
void read_acc_data_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm);
void acc_detect_init(struct sensor_detect_manager *sm, uint32_t len);

#endif
