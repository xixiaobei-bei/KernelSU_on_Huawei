/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: gyro sysfs header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __GYRO_SYSFS_H__
#define __GYRO_SYSFS_H__

#include "gyro_channel.h"
#include "gyro_detect.h"
#include "gyro_route.h"

#define GYRO1_CALIDATA_LEN 15

struct gyro_offset_threshold {
	int32_t low_threshold;
	int32_t high_threshold;
};

ssize_t attr_gyro_calibrate_show(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t attr_gyro1_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t attr_gyro_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t attr_get_gyro_sensor_id(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t show_gyro_set_calidata(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t store_gyro_set_calidata(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t sensors_calibrate_show_gyro(int tag, struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t show_gyro1_set_calidata(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t store_gyro1_set_calidata(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);

#endif
