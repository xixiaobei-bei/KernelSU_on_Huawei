/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als sysfs header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ALS_SYSFS_H__
#define __ALS_SYSFS_H__

#include "als_channel.h"
#include "als_detect.h"
#include "als_gamma_config.h"
#include "als_para_debug.h"
#include "als_route.h"

int get_stop_auto_als(void);
ssize_t attr_als_calibrate_show(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t attr_als1_calibrate_show(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t attr_als2_calibrate_show(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t attr_als_calibrate_write_by_tag(int32_t tag, struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t attr_als_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t attr_als1_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t attr_als2_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t attr_set_stop_als_auto_data(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t attr_set_dt_als_sensor_stup(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t attr_get_als_sensor_id(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t sensors_calibrate_show_als(int tag, struct device *dev,
	struct device_attribute *attr, char *buf);

#endif
