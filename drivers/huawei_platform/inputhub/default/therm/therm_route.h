/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: thermometer route header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __THERM_ROUTE_H__
#define __THERM_ROUTE_H__

#include "therm_detect.h"

ssize_t therm_store_set_mode(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);

#endif
