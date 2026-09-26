/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: gyro detect header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __GYRO_DETECT_H__
#define __GYRO_DETECT_H__

#include "sensor_sysfs.h"

#define GYRO_DEV_COUNT_MAX 2

#define GYRO_OFFSET_CALIBRATE_LENGTH 3

#define GYRO_STATIC_CALI_LENGTH 3
#define GYRO_DYN_CALIBRATE_END_ORDER  5

struct gyro_platform_data {
	struct sensor_combo_cfg cfg;
	uint8_t exist;
	uint8_t position;
	uint8_t axis_map_x;
	uint8_t axis_map_y;
	uint8_t axis_map_z;
	uint8_t negate_x;
	uint8_t negate_y;
	uint8_t negate_z;
	GPIO_NUM_TYPE gpio_int1;
	GPIO_NUM_TYPE gpio_int2;
	GPIO_NUM_TYPE gpio_int2_sh;
	uint16_t poll_interval;
	uint8_t fac_fix_offset_y;
	uint8_t still_calibrate_threshold;
	uint8_t calibrate_way;
	uint16_t calibrate_thredhold;
	uint16_t gyro_range;
	uint8_t gyro_extend_data[SENSOR_PLATFORM_EXTEND_DATA_SIZE];
};

struct gyro_device_info {
	int32_t obj_tag;
	uint32_t detect_list_id;
	uint8_t gyro_dev_index;
	uint8_t gyro_first_start_flag;
	uint8_t gyro_opened_before_calibrate;
	int32_t gyro_calib_data[CALIBRATE_DATA_LENGTH];
	enum ret_type gyro_calibration_res;
};

struct gyro_platform_data *gyro_get_platform_data(int32_t tag);
struct gyro_device_info *gyro_get_device_info(int32_t tag);
char *gyro_get_sensors_id_string(void);
void gyro_get_sensors_id_from_dts(struct device_node *dn);
void read_gyro_data_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm);
int gyro1_data_from_mcu(const struct pkt_header *head);
int gyro_data_from_mcu(const struct pkt_header *head);
void gyro_detect_init(struct sensor_detect_manager *sm, uint32_t len);

#endif
