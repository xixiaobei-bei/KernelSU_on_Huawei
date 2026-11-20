/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: airpress route header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __AIRPRESS_ROUTE_H__
#define __AIRPRESS_ROUTE_H__

#include "airpress_detect.h"

ssize_t show_airpress_sensorlist_info(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t attr_airpress_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t attr_airpress_calibrate_show(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t attr_airpress_set_tp_info_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);


#endif
