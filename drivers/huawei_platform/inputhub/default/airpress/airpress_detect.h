/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: airpress detect header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __AIRPRESS_DETECT_H__
#define __AIRPRESS_DETECT_H__

#include "sensor_sysfs.h"

#define AIRPRESS_DEV_COUNT_MAX 1

#define TP_COORDINATE_THRESHOLD 4

struct airpress_platform_data {
	struct sensor_combo_cfg cfg;
	int offset;
	int is_support_touch;
	uint16_t poll_interval;
	uint16_t touch_fac_order;
	uint16_t touch_fac_wait_time;
	uint16_t tp_touch_coordinate_threshold[TP_COORDINATE_THRESHOLD];
	uint8_t airpress_extend_data[SENSOR_PLATFORM_EXTEND_DATA_SIZE];
};

struct airpress_device_info {
	int32_t obj_tag;
	uint32_t detect_list_id;
	uint8_t airpress_dev_index;
};

struct airpress_platform_data *airpress_get_platform_data(int32_t tag);
struct airpress_device_info *airpress_get_device_info(int32_t tag);
void read_airpress_data_from_dts(struct device_node *dn);
int airpress_sensor_detect(struct device_node *dn,
	struct sensor_detect_manager *sm, int index);
void airpress_detect_init(struct sensor_detect_manager *sm, uint32_t len);

#endif
