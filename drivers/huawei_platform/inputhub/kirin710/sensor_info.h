/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: sensor info header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __SENSOR_INFO_H__
#define __SENSOR_INFO_H__

#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif

#include "sensor_sysfs.h"

int get_stop_auto_motion(void);
int get_flag_for_sensor_test(void);
int get_stop_auto_accel(void);
ssize_t attr_set_gsensor_gather_enable(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t attr_set_sensor_test_mode(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t attr_set_dt_motion_stup(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t attr_set_stop_auto_data(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t attr_stop_auto_motion_show(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t attr_set_stop_auto_motion(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t attr_set_sensor_stepcounter_stup(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);

#endif
