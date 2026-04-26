/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para debug header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ALS_PARA_DEBUG_H__
#define __ALS_PARA_DEBUG_H__

#include "als_detect.h"

ssize_t als_debug_data_show(int32_t tag, struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t als_debug_data_store(int32_t tag, struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);

#endif

