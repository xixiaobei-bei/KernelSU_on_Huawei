/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: cap prox sysfs header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __CAP_PROX_SYSFS_H__
#define __CAP_PROX_SYSFS_H__

#include "cap_prox_channel.h"
#include "cap_prox_detect.h"
#include "cap_prox_route.h"

ssize_t attr_cap_prox1_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t attr_cap_prox_calibrate_show(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t attr_cap_prox_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t show_cap_prox_calibrate_method(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t show_cap_prox_calibrate_orders(int tag, struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t attr_get_cap_sensor_id(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t show_sar_data(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t sensors_calibrate_show_cap_prox(int tag, struct device *dev,
	struct device_attribute *attr, char *buf);

#endif
