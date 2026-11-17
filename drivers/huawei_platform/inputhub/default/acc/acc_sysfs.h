/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: acc sysfs header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ACC_SYSFS_H__
#define __ACC_SYSFS_H__

#include "acc_channel.h"
#include "acc_detect.h"
#include "acc_route.h"

#define ACC1_CALIDATA_LEN 15

struct acc_offset_threshold {
	int32_t low_threshold;
	int32_t high_threshold;
};

ssize_t attr_acc_calibrate_show(struct device *dev,
	struct device_attribute *attr, char *buf);
int acc1_calibrate_save(const void *buf, int length);
int acc_calibrate_save(const void *buf, int length);
ssize_t attr_acc1_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t attr_acc_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t attr_get_acc_sensor_id(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t show_acc_set_calidata(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t store_acc_set_calidata(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t sensors_calibrate_show_acc(int tag, struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t show_acc1_set_calidata(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t store_acc1_set_calidata(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);

#endif
