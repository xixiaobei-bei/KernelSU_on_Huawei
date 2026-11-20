/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: thermometer sysfs header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __THERM_SYSFS_H__
#define __THERM_SYSFS_H__

#include "therm_channel.h"
#include "therm_detect.h"
#include "therm_route.h"

ssize_t attr_thermometer_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t attr_get_therm_sensor_id(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t attr_get_therm_calibrate_index(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t sensors_calibrate_show_therm(int tag, struct device *dev,
	struct device_attribute *attr, char *buf);

#endif
