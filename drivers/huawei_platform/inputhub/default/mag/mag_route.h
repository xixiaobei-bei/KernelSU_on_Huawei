/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: mag route header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __MAG_ROUTE_H__
#define __MAG_ROUTE_H__

#include "mag_detect.h"

ssize_t show_mag_calibrate_method(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t show_mag_sensorlist_info(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t calibrate_threshold_from_mag_show(struct device *dev,
	struct device_attribute *attr, char *buf);


#endif
