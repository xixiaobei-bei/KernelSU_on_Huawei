/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: mag sysfs header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __MAG_SYSFS_H__
#define __MAG_SYSFS_H__

#include "mag_channel.h"
#include "mag_detect.h"
#include "mag_route.h"

ssize_t attr_get_mag_sensor_id(struct device *dev,
	struct device_attribute *attr, char *buf);

#endif
