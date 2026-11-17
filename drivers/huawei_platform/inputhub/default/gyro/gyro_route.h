/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: gyro route header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __GYRO_ROUTE_H__
#define __GYRO_ROUTE_H__

#include "gyro_detect.h"

ssize_t show_gyro_sensorlist_info(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t show_gyro_position_info(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t store_ungyro_time_offset(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t show_ungyro_time_offset(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t show_gyro_offset_data(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t show_gyro1_offset_data(struct device *dev,
	struct device_attribute *attr, char *buf);

#endif
