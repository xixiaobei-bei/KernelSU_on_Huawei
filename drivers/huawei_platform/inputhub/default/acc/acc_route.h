/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: acc route header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ACC_ROUTE_H__
#define __ACC_ROUTE_H__

#include "acc_detect.h"

int send_acc_filter_flag(unsigned long value);
ssize_t show_acc_sensorlist_info(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t store_unacc_time_offset(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t show_unacc_time_offset(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t show_acc_offset_data(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t show_acc1_offset_data(struct device *dev,
	struct device_attribute *attr, char *buf);

#endif
